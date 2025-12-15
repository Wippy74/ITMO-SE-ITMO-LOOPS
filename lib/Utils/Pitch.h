#pragma once

#include <string>

namespace utils {

class Pitch {
public:
  static int ToMidi(const std::string& pitch);
  static double MidiToHz(int midi);
};

} // namespace utils
