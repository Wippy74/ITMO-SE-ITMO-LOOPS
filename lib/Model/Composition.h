#pragma once

#include <string>
#include <unordered_map>

#include "InstrumentSpec.h"
#include "Pattern.h"

namespace model {

struct Composition {
  int bpm;
  std::unordered_map<std::string, InstrumentSpec> instruments;
  std::unordered_map<std::string, Pattern> patterns;
};

} // namespace model
