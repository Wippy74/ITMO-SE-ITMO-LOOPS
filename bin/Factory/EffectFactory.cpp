#include "../Factory/EffectFactory.h"

#include <stdexcept>
#include <unordered_map>

namespace factory {

static std::unordered_map<std::string, EffectFactory::Creator>& registry() {
  static std::unordered_map<std::string, EffectFactory::Creator> r;
  return r;
}

bool EffectFactory::RegisterType(const std::string& type, Creator creator) {
  if (type.empty()) {
    throw std::runtime_error("EffectFactory: type must not be empty");
  }
  if (!creator) {
    throw std::runtime_error("EffectFactory: creator must not be null");
  }
  auto& r = registry();
  auto [it, inserted] = r.emplace(type, std::move(creator));
  return inserted;
}

std::unique_ptr<effects::IEffect> EffectFactory::create(const model::EffectSpec& spec) {
  auto& r = registry();
  auto it = r.find(spec.type);
  if (it == r.end()) {
    throw std::runtime_error("EffectFactory: unknown effect type '" + spec.type + "'");
  }
  return (it->second)(spec);
}

bool EffectFactory::IsRegistered(const std::string& type) {
  auto& r = registry();
  return r.find(type) != r.end();
}

void EffectFactory::clear() {
  registry().clear();
}

} // namespace factory
