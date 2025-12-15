#pragma once

#include <string>
#include <unordered_map>
#include <vector>

namespace model {

using Params = std::unordered_map<std::string, std::string>;

struct EffectSpec {
  std::string type;
  Params params;
};

struct InstrumentSpec {
  std::string name;
  std::string type;
  Params params;
  std::vector<EffectSpec> effects;
};

} // namespace model
