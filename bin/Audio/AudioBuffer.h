#pragma once

#include <cstddef>
#include <vector>

namespace audio {

struct AudioBuffer {
  std::vector<float> data;
  size_t samples() const;
  void clear();
  void resize(size_t n, float value = 0.0f);
  void EnsureSize(size_t n, float value = 0.0f);
  void MixAddFrom(const AudioBuffer& src, size_t offset);
  void ApplyGain(float g);
  void clip(float lo = -1.0f, float hi = 1.0f);
};

} // namespace audio
