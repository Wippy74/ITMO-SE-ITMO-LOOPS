#include "../Factory/InstrumentFactory.h"

#include <stdexcept>
#include <unordered_map>

namespace factory {

static std::unordered_map<std::string, InstrumentFactory::Creator>& registry() {
  static std::unordered_map<std::string, InstrumentFactory::Creator> r;
  return r;
}

bool InstrumentFactory::RegisterType(const std::string& type, Creator creator) {
  if (type.empty()) {
    throw std::runtime_error("InstrumentFactory: type must not be empty");
  }
  if (!creator) {
    throw std::runtime_error("InstrumentFactory: creator must not be null");
  }
  auto& r = registry();
  auto [it, inserted] = r.emplace(type, std::move(creator));
  return inserted;
}

std::unique_ptr<instruments::IInstrument> InstrumentFactory::create(const model::InstrumentSpec& spec) {
  auto& r = registry();
  auto it = r.find(spec.type);
  if (it == r.end()) {
    throw std::runtime_error("InstrumentFactory: unknown instrument type '" + spec.type + "'");
  }
  return (it->second)(spec);
}

bool InstrumentFactory::IsRegistered(const std::string& type) {
  auto& r = registry();
  return r.find(type) != r.end();
}

void InstrumentFactory::clear() {
  registry().clear();
}

} // namespace factory
