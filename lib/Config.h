#pragma once

namespace itmoloops {

struct Config {
  static constexpr int kSampleRate = 44100;
  static constexpr double kTailSec = 2.0;
  static constexpr bool kAutoNormalize = true;
  static constexpr const char* kEntryPattern = "main";
};

} // namespace itmoloops
