#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>

#include "IEffect.h"
#include "../Factory/EffectFactory.h"
#include "../Model/InstrumentSpec.h"

namespace effects {

static double GetFloat(const model::Params& p, const std::string& key, double def) {
  auto it = p.find(key);
  if (it == p.end()) return def;
  try {
    return std::stod(it->second);
  } catch (...) {
    throw std::runtime_error("TremoloEffect: invalid float param '" + key + "'");
  }
}

class TremoloEffect final : public IEffect {
public:
  explicit TremoloEffect(const model::EffectSpec& spec) { 
    m_freq = GetFloat(spec.params, "freq", 5.0);
    m_depth = GetFloat(spec.params, "depth", 0.5);
    if (m_freq < 0.0) {
      throw std::runtime_error("TremoloEffect: freq must be >= 0");
    }
    if (m_depth < 0.0) {
      m_depth = 0.0;
    }
    if (m_depth > 1.0) {
      m_depth = 1.0;
    }
  }
  void process(audio::AudioBuffer& buffer, int SampleRate) override {
    if (SampleRate <= 0) {
      throw std::runtime_error("TremoloEffect: SampleRate must be > 0");
    }
    if (buffer.data.empty()) {
      return;
    }
    const double twoPi = 2.0 * 3.14159265358979323846;
    const double invSr = 1.0 / static_cast<double>(SampleRate);
    for (size_t n = 0; n < buffer.data.size(); ++n) {
      const double t = static_cast<double>(n) * invSr;
      const double mod = (1.0 - m_depth) + m_depth * std::sin(twoPi * m_freq * t);
      buffer.data[n] = static_cast<float>(static_cast<double>(buffer.data[n]) * mod);
    }
  }

private:
  double m_freq = 5.0;
  double m_depth = 0.5;
};

} // namespace effects

namespace {
struct TremoloEffectRegistrar {
  TremoloEffectRegistrar() {
    factory::EffectFactory::RegisterType(
      "tremolo",
      [](const model::EffectSpec& spec) {
        return std::make_unique<effects::TremoloEffect>(spec);
      }
    );
  }
};

static TremoloEffectRegistrar g_tremolo_registrar;
} // namespace
