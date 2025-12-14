#include "AudioBuffer.h"

namespace audio {

size_t AudioBuffer::samples() const {
  return data.size();
}

void AudioBuffer::clear() {
  data.clear();
}

void AudioBuffer::resize(size_t n, float value) {
  data.resize(n, value);
}

void AudioBuffer::EnsureSize(size_t n, float value) {
  if (data.size() < n) {
    data.resize(n, value);
  }
}

void AudioBuffer::MixAddFrom(const AudioBuffer& src, size_t offset) {
  if (src.data.empty()) {
    return;
  }
  EnsureSize(offset + src.data.size(), 0.0f);
  for (size_t i = 0; i < src.data.size(); ++i) {
    data[offset + i] += src.data[i];
  }
}

void AudioBuffer::ApplyGain(float g) {
  for (float& x : data) x *= g;
}

void AudioBuffer::clip(float lo, float hi) {
  for (float& x : data) {
    if (x < lo) {
      x = lo;
    }
    else if (x > hi) {
      x = hi;
    }
  }
}

} // namespace audio
