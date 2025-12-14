#pragma once

#include "../Audio/AudioBuffer.h"

namespace effects {

class IEffect {
public:
  virtual ~IEffect() = default;
  virtual void process(audio::AudioBuffer& buffer, int SampleRate) = 0;
};

} // namespace effects
