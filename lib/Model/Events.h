#pragma once

#include <string>
#include <vector>

namespace model {

struct NoteEvent {
  double startSec;
  double durSec;
  int midi = 60;
  float velocity01 = 1.0f;
  std::string instrument;
};

using NoteEvents = std::vector<NoteEvent>;

} // namespace model
