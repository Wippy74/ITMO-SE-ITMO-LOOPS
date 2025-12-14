#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

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
    throw std::runtime_error("EchoEffect: invalid float param '" + key + "'");
  }
}

class EchoEffect final : public IEffect {
public:
  explicit EchoEffect(const model::EffectSpec& spec) {
    m_delaySec = GetFloat(spec.params, "delay", 0.0);
    m_decay = GetFloat(spec.params, "decay", 0.0);
    if (m_delaySec < 0.0) {
      throw std::runtime_error("EchoEffect: delay must be >= 0");
    }
  }

  void process(audio::AudioBuffer& buffer, int SampleRate) override {
    if (SampleRate <= 0) {
      throw std::runtime_error("EchoEffect: SampleRate must be > 0");
    }
    if (buffer.data.empty()) {
      return;
    }
    const size_t delaySamples = static_cast<size_t>(std::llround(m_delaySec * SampleRate));
    if (delaySamples == 0 || m_decay == 0.0) {
      return;
    }
    const std::vector<float> in = buffer.data;
    const size_t inN = in.size();
    const size_t outN = inN + delaySamples;
    std::vector<float> out(outN, 0.0f);
    for (size_t n = 0; n < inN; ++n) {
      out[n] += in[n];
      out[n + delaySamples] += static_cast<float>(m_decay) * in[n];
    }
    buffer.data = std::move(out);
  }

private:
  double m_delaySec = 0.0;
  double m_decay = 0.0;
};

} // namespace effects


namespace {
struct EchoEffectRegistrar {
  EchoEffectRegistrar() {
    factory::EffectFactory::RegisterType(
      "echo",
      [](const model::EffectSpec& spec) {
        return std::make_unique<effects::EchoEffect>(spec);
      }
    );
  }
};
static EchoEffectRegistrar g_echo_registrar;
} // namespace
