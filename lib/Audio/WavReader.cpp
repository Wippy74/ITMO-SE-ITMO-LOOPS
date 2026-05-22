#include "WavIO.h"

#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace audio {

static uint16_t ReadLE16(std::istream& in) {
  uint8_t b0 = 0;
  uint8_t b1 = 0;
  in.read(reinterpret_cast<char*>(&b0), 1);
  in.read(reinterpret_cast<char*>(&b1), 1);
  if (!in) {
    throw std::runtime_error("WavReader: unexpected EOF while reading u16");
  }
  return static_cast<uint16_t>(b0 | (static_cast<uint16_t>(b1) << 8));
}

static uint32_t ReadLE32(std::istream& in) {
  uint8_t b0 = 0;
  uint8_t b1 = 0;
  uint8_t b2 = 0;
  uint8_t b3 = 0;
  in.read(reinterpret_cast<char*>(&b0), 1);
  in.read(reinterpret_cast<char*>(&b1), 1);
  in.read(reinterpret_cast<char*>(&b2), 1);
  in.read(reinterpret_cast<char*>(&b3), 1);
  if (!in) {
    throw std::runtime_error("WavReader: unexpected EOF while reading u32");
  }
  return static_cast<uint32_t>(b0 | (static_cast<uint32_t>(b1) << 8) | (static_cast<uint32_t>(b2) << 16) | (static_cast<uint32_t>(b3) << 24));
}

static std::string ReadFourCC(std::istream& in) {
  char cc[4] = {0, 0, 0, 0};
  in.read(cc, 4);
  if (!in) {
    throw std::runtime_error("WavReader: unexpected EOF while reading FourCC");
  }
  return std::string(cc, 4);
}

static float int16ToFloat(int16_t v) {
  if (v == static_cast<int16_t>(-32768)) {
    return -1.0f;
  }
  return static_cast<float>(v) / 32768.0f;
}

AudioBuffer WavReader::ReadWav(const std::string& path) {
  std::ifstream in(path, std::ios::binary);
  if (!in.is_open()) {
    throw std::runtime_error("WavReader: cannot open file: " + path);
  }
  const std::string riff = ReadFourCC(in);
  if (riff != "RIFF") {
    throw std::runtime_error("WavReader: not a RIFF file");
  }
  (void)ReadLE32(in);
  const std::string wave = ReadFourCC(in);
  if (wave != "WAVE") {
    throw std::runtime_error("WavReader: not a WAVE file");
  }
  bool haveFmt = false;
  bool haveData = false;
  uint16_t audioFormat = 0;
  uint16_t numChannels = 0;
  uint32_t SampleRate = 0;
  uint16_t bitsPerSample = 0;
  std::vector<uint8_t> rawData;
  while (in && !(haveFmt && haveData)) {
    if (in.peek() == EOF) {
      break;
    }
    const std::string chunkId = ReadFourCC(in);
    const uint32_t chunkSize = ReadLE32(in);
    if (chunkId == "fmt ") {
      if (chunkSize < 16) {
        throw std::runtime_error("WavReader: invalid fmt chunk size");
      }
      audioFormat = ReadLE16(in);
      numChannels = ReadLE16(in);
      SampleRate = ReadLE32(in);
      (void)ReadLE32(in);
      (void)ReadLE16(in);
      bitsPerSample = ReadLE16(in);
      const uint32_t extra = chunkSize - 16;
      if (extra > 0) {
        in.ignore(static_cast<std::streamsize>(extra));
      }
      haveFmt = true;
    } else if (chunkId == "data") {
      rawData.resize(chunkSize);
      if (chunkSize > 0) {
        in.read(reinterpret_cast<char*>(rawData.data()), static_cast<std::streamsize>(chunkSize));
        if (!in) {
          throw std::runtime_error("WavReader: unexpected EOF in data chunk");
        }
      }
      haveData = true;
    } else {
      in.ignore(static_cast<std::streamsize>(chunkSize));
    }
    if (chunkSize % 2 == 1) {
      in.ignore(1);
    }
  }
  if (!haveFmt) {
    throw std::runtime_error("WavReader: missing fmt chunk");
  }
  if (!haveData) {
    throw std::runtime_error("WavReader: missing data chunk");
  }
  if (audioFormat != 1) {
    throw std::runtime_error("WavReader: only PCM (format=1) supported");
  }
  if (rawData.size() % 2 != 0) {
    throw std::runtime_error("WavReader: invalid PCM16 data size");
  }
  const size_t samples = rawData.size() / 2;
  AudioBuffer out;
  out.data.resize(samples);
  for (size_t i = 0; i < samples; ++i) {
    const uint8_t lo = rawData[i * 2 + 0];
    const uint8_t hi = rawData[i * 2 + 1];
    const int16_t v = static_cast<int16_t>(
      static_cast<uint16_t>(lo | (static_cast<uint16_t>(hi) << 8))
    );
    out.data[i] = int16ToFloat(v);
  }

  return out;
}

} // namespace audio
