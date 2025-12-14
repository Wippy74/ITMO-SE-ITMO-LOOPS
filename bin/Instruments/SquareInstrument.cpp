#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>

#include "../Factory/InstrumentFactory.h"
#include "IInstrument.h"
#include "../Model/InstrumentSpec.h"

namespace instruments {

static double GetFloat(const model::Params& p, const std::string& key, double def) {
  auto it = p.find(key);
  if (it == p.end()) return def;
  try {
    return std::stod(it->second);
  } catch (...) {
    throw std::runtime_error("SquareInstrument: invalid float param '" + key + "'");
  }
}

static int GetInt(const model::Params& p, const std::string& key, int def) {
  auto it = p.find(key);
  if (it == p.end()) return def;
  try {
    size_t idx = 0;
    int v = std::stoi(it->second, &idx, 10);
    if (idx != it->second.size()) throw std::runtime_error("");
    return v;
  } catch (...) {
    throw std::runtime_error("SquareInstrument: invalid int param '" + key + "'");
  }
}

static void ApplyAttackRelease(audio::AudioBuffer& b, int sr, double attackSec, double releaseSec) {
  const size_t n = b.data.size();
  if (n == 0) {
    return;
  }
  size_t aN = (attackSec > 0.0) ? static_cast<size_t>(std::llround(attackSec * sr)) : 0;
  size_t rN = (releaseSec > 0.0) ? static_cast<size_t>(std::llround(releaseSec * sr)) : 0;
  if (aN > n) {
    aN = n;
  }
  if (rN > n) {
    rN = n;
  }
  for (size_t i = 0; i < n; ++i) {
    float envA = 1.0f;
    float envR = 1.0f;
    if (aN > 0 && i < aN) {
      envA = static_cast<float>(static_cast<double>(i) / static_cast<double>(aN));
    }
    if (rN > 0 && i >= n - rN) {
      const size_t k = i - (n - rN);
      envR = static_cast<float>(1.0 - static_cast<double>(k) / static_cast<double>(rN));
    }
    const float env = (envA < envR) ? envA : envR;
    b.data[i] *= env;
  }
}

class SquareInstrument final : public IInstrument {
public:
  explicit SquareInstrument(const model::InstrumentSpec& spec) {
    m_attack = GetFloat(spec.params, "attack", 0.0);
    m_release = GetFloat(spec.params, "release", 0.0);
    int duty = GetInt(spec.params, "duty", 50);
    if (duty < 0) {
      duty = 0;
    }
    if (duty > 100) {
      duty = 100;
    }
    m_duty01 = static_cast<double>(duty) / 100.0;
    if (m_attack < 0.0 || m_release < 0.0) {
      throw std::runtime_error("SquareInstrument: attack/release must be >= 0");
    }
  }

  audio::AudioBuffer RenderNote(int midi, double durSec, float velocity01, int SampleRate) override {
    audio::AudioBuffer out;
    if (durSec <= 0.0 || SampleRate <= 0) {
      return out;
    }
    if (velocity01 < 0.0f) {
      velocity01 = 0.0f;
    }
    if (velocity01 > 1.0f) {
      velocity01 = 1.0f;
    }
    const size_t n = static_cast<size_t>(std::ceil(durSec * SampleRate));
    out.data.resize(n, 0.0f);
    const double f = MidiToHz(midi);
    const double invSr = 1.0 / static_cast<double>(SampleRate);
    for (size_t i = 0; i < n; ++i) {
      const double t = static_cast<double>(i) * invSr;
      const double phase = f * t;
      const double phase01 = phase - std::floor(phase); // [0..1)
      const float s = (phase01 < m_duty01) ? 1.0f : -1.0f;
      out.data[i] = s * velocity01;
    }
    ApplyAttackRelease(out, SampleRate, m_attack, m_release);
    return out;
  }

private:
  double m_attack = 0.0;
  double m_release = 0.0;
  double m_duty01 = 0.5;
};

} // namespace instruments

namespace {
struct SquareInstrumentRegistrar {
  SquareInstrumentRegistrar() {
    factory::InstrumentFactory::RegisterType(
      "square",
      [](const model::InstrumentSpec& spec) {
        return std::make_unique<instruments::SquareInstrument>(spec);
      }
    );
  }
};
static SquareInstrumentRegistrar g_square_registrar;
} // namespace
