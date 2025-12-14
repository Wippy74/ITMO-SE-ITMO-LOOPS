#pragma once

#include <cmath>

#include "../Audio/AudioBuffer.h"

namespace instruments {

class IInstrument {
public:
  virtual ~IInstrument() = default;
  virtual audio::AudioBuffer RenderNote(int midi, double durSec, float velocity01, int SampleRate) = 0;
  static double MidiToHz(int midi) {
    return 440.0 * std::pow(2.0, (static_cast<double>(midi) - 69.0) / 12.0);
  }
};

} // namespace instruments
