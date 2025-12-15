#pragma once

#include <functional>
#include <memory>
#include <string>

#include "../Effects/IEffect.h"
#include "../Model/InstrumentSpec.h" 

namespace factory {

class EffectFactory {
public:
  using Creator = std::function<std::unique_ptr<effects::IEffect>(const model::EffectSpec&)>;
  static bool RegisterType(const std::string& type, Creator creator);
  static std::unique_ptr<effects::IEffect> create(const model::EffectSpec& spec);
  static bool IsRegistered(const std::string& type);
  static void clear();

private:
  EffectFactory() = default;
};

} // namespace factory
