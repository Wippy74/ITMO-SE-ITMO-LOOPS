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
    throw std::runtime_error("GainEffect: invalid float param '" + key + "'");
  }
}

class GainEffect final : public IEffect {
public:
  explicit GainEffect(const model::EffectSpec& spec) {
    m_gain = GetFloat(spec.params, "gain", 1.0);
  }
  void process(audio::AudioBuffer& buffer, int SampleRate) override {
    buffer.ApplyGain(static_cast<float>(m_gain));
  }

private:
  double m_gain = 1.0;
};

} // namespace effects

namespace {
struct GainEffectRegistrar {
  GainEffectRegistrar() {
    factory::EffectFactory::RegisterType(
      "gain",
      [](const model::EffectSpec& spec) {
        return std::make_unique<effects::GainEffect>(spec);
      }
    );
  }
};

static GainEffectRegistrar g_gain_registrar;
} // namespace
