#pragma once

#include <string>
#include <vector>

#include "Commands.h"

namespace model {

struct Pattern {
  std::string name;
  int resolution;
  std::vector<Command> commands;
};

} // namespace model
