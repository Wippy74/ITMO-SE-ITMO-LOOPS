#include "Pitch.h"
#include "Errors.h"

#include <cmath>
#include <stdexcept>


namespace utils {

static int BaseSemitone(char note) {
  switch (note) {
    case 'C': return 0;
    case 'D': return 2;
    case 'E': return 4;
    case 'F': return 5;
    case 'G': return 7;
    case 'A': return 9;
    case 'B': return 11;
    default: return -1000;
  }
}

int Pitch::ToMidi(const std::string& pitch) {
  if (pitch.size() < 2) {
    throw ConfigError("Invalid pitch '" + pitch + "'");
  } 
  const char n = pitch[0];
  int semi = BaseSemitone(n);
  if (semi < -100) {
    throw ConfigError("Invalid pitch note letter '" + pitch + "'");
  }
  size_t i = 1;
  if (i < pitch.size() && (pitch[i] == '#' || pitch[i] == 'b')) {
    semi += (pitch[i] == '#') ? 1 : -1;
    ++i;
  }
  if (i >= pitch.size()) {
    throw ConfigError("Invalid pitch octave in '" + pitch + "'");
  }
  int octave = 0;
  try {
    octave = std::stoi(pitch.substr(i));
  } catch (...) {
    throw ConfigError("Invalid pitch octave in '" + pitch + "'");
  }
  const int midi = (octave + 1) * 12 + semi;
  if (midi < 0 || midi > 127) {
    throw ConfigError("Pitch '" + pitch + "' out of MIDI range");
  }
  return midi;
}

double Pitch::MidiToHz(int midi) {
  if (midi < 0 || midi > 127) {
    throw ConfigError("MIDI note out of range");
  }
  return 440.0 * std::pow(2.0, (static_cast<double>(midi) - 69.0) / 12.0);
}

} // namespace utils
