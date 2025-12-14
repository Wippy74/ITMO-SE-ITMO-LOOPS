#include "WavIO.h"

#include <cstdint>
#include <fstream>
#include <stdexcept>

namespace audio {

static void writeFourCC(std::ostream& out, const char cc[4]) {
  out.write(cc, 4);
}

static void writeLE16(std::ostream& out, uint16_t v) {
  uint8_t b0 = static_cast<uint8_t>(v & 0xFF);
  uint8_t b1 = static_cast<uint8_t>((v >> 8) & 0xFF);
  out.write(reinterpret_cast<const char*>(&b0), 1);
  out.write(reinterpret_cast<const char*>(&b1), 1);
}

static void writeLE32(std::ostream& out, uint32_t v) {
  uint8_t b0 = static_cast<uint8_t>(v & 0xFF);
  uint8_t b1 = static_cast<uint8_t>((v >> 8) & 0xFF);
  uint8_t b2 = static_cast<uint8_t>((v >> 16) & 0xFF);
  uint8_t b3 = static_cast<uint8_t>((v >> 24) & 0xFF);
  out.write(reinterpret_cast<const char*>(&b0), 1);
  out.write(reinterpret_cast<const char*>(&b1), 1);
  out.write(reinterpret_cast<const char*>(&b2), 1);
  out.write(reinterpret_cast<const char*>(&b3), 1);
}

static int16_t floatToInt16(float x) {
  if (x > 1.0f) {
    x = 1.0f;
  }
  if (x < -1.0f) {
    x = -1.0f;
  }
  if (x <= -1.0f) {
    return static_cast<int16_t>(-32768);
  }
  if (x >= 1.0f) {
    return static_cast<int16_t>(32767);
  }
  return static_cast<int16_t>(x * 32767.0f);
}

void WavWriter::WriteWav(const std::string& path, const AudioBuffer& buffer) {
  std::ofstream out(path, std::ios::binary);
  if (!out.is_open()) {
    throw std::runtime_error("WavWriter: cannot open file for writing: " + path);
  }
  const uint16_t audioFormat = 1;
  const uint16_t numChannels = 1;
  const uint32_t SampleRate = 44100;
  const uint16_t bitsPerSample = 16;
  const uint16_t blockAlign = numChannels * (bitsPerSample / 8);
  const uint32_t byteRate = SampleRate * blockAlign;
  const uint32_t fmtChunkSize = 16;
  const uint32_t dataBytes = static_cast<uint32_t>(buffer.data.size() * sizeof(int16_t));
  const uint32_t riffSize = 4 + (8 + fmtChunkSize) + (8 + dataBytes);
  writeFourCC(out, "RIFF");
  writeLE32(out, riffSize);
  writeFourCC(out, "WAVE");
  writeFourCC(out, "fmt ");
  writeLE32(out, fmtChunkSize);
  writeLE16(out, audioFormat);
  writeLE16(out, numChannels);
  writeLE32(out, SampleRate);
  writeLE32(out, byteRate);
  writeLE16(out, blockAlign);
  writeLE16(out, bitsPerSample);
  writeFourCC(out, "data");
  writeLE32(out, dataBytes);
  for (float x : buffer.data) {
    const int16_t s = floatToInt16(x);
    writeLE16(out, static_cast<uint16_t>(static_cast<int16_t>(s)));
  }
  if (!out) {
    throw std::runtime_error("WavWriter: write failed");
  }
}

} // namespace audio
