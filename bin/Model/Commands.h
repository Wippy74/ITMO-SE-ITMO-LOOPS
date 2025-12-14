#pragma once

#include <string>
#include <variant>

namespace model {

struct NoteCmd {
  int startUnits;
  std::string instrument;
  std::string pitch;
  int durUnits;
  int velocity;
};

struct CallCmd {
  int startUnits;
  std::string patternName;
};

using Command = std::variant<NoteCmd, CallCmd>;

} // namespace model
