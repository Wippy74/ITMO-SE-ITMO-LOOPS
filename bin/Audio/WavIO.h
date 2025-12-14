#pragma once

#include <string>

#include "AudioBuffer.h"

namespace audio {

class WavReader {
public:
  static AudioBuffer ReadWav(const std::string& path);
};

class WavWriter {
public:
  static void WriteWav(const std::string& path, const AudioBuffer& buffer);
};

} // namespace udio
