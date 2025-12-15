#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "../Audio/WavIO.h"
#include "../Factory/InstrumentFactory.h"
#include "IInstrument.h"
#include "../Model/InstrumentSpec.h"

namespace instruments {

static std::string GetString(const model::Params& p, const std::string& key, const std::string& def) {
  auto it = p.find(key);
  if (it == p.end()) {
    return def;
  }
  return it->second;
}

static double GetFloat(const model::Params& p, const std::string& key, double def) {
  auto it = p.find(key);
  if (it == p.end()) {
    return def;
  }
  try {
    return std::stod(it->second);
  } catch (...) {
    throw std::runtime_error("SamplerInstrument: invalid float param '" + key + "'");
  }
}

static bool TryParseLoop(const model::Params& p, size_t& loopStart, size_t& loopEndInclusive) {
  auto it = p.find("loop");
  if (it == p.end()) {
    return false;
  }
  const std::string& s = it->second;
  const auto comma = s.find(',');
  if (comma == std::string::npos) {
    throw std::runtime_error("SamplerInstrument: loop must be 'start,end'");
  }
  try {
    long long a = std::stoll(s.substr(0, comma));
    long long b = std::stoll(s.substr(comma + 1));
    if (a < 0 || b < 0) {
      throw std::runtime_error("");
    }
    loopStart = static_cast<size_t>(a);
    loopEndInclusive = static_cast<size_t>(b);
    return true;
  } catch (...) {
    throw std::runtime_error("SamplerInstrument: invalid loop value '" + s + "'");
  }
}


static int PitchToMidi(const std::string& pitch) {
  if (pitch.size() < 2) {
    throw std::runtime_error("SamplerInstrument: invalid root pitch '" + pitch + "'");
  }
  auto noteBase = [](char c) -> int {
    switch (c) {
      case 'C': return 0;
      case 'D': return 2;
      case 'E': return 4;
      case 'F': return 5;
      case 'G': return 7;
      case 'A': return 9;
      case 'B': return 11;
      default: return -1000;
    }
  };
  int semitone = noteBase(pitch[0]);
  if (semitone < -100) {
    throw std::runtime_error("SamplerInstrument: invalid note letter in '" + pitch + "'");
  }
  size_t i = 1;
  if (i < pitch.size() && (pitch[i] == '#' || pitch[i] == 'b')) {
    semitone += (pitch[i] == '#') ? 1 : -1;
    ++i;
  }
  if (i >= pitch.size()) {
    throw std::runtime_error("SamplerInstrument: missing octave in '" + pitch + "'");
  }
  int octave = 0;
  try {
    octave = std::stoi(pitch.substr(i));
  } catch (...) {
    throw std::runtime_error("SamplerInstrument: invalid octave in '" + pitch + "'");
  }
  const int midi = (octave + 1) * 12 + semitone;
  if (midi < 0 || midi > 127) {
    throw std::runtime_error("SamplerInstrument: root pitch out of MIDI range '" + pitch + "'");
  }
  return midi;
}

static void ApplyAttackRelease(audio::AudioBuffer& b, int sr, double attackSec, double releaseSec) {
  const size_t n = b.data.size();
  if (n == 0) {
    return;
  }
  size_t aN = (attackSec > 0.0) ? static_cast<size_t>(std::llround(attackSec * sr)) : 0;
  size_t rN = (releaseSec > 0.0) ? static_cast<size_t>(std::llround(releaseSec * sr)) : 0;
  if (aN > n) {
    aN = n;
  }
  if (rN > n) {
    rN = n;
  }
  for (size_t i = 0; i < n; ++i) {
    float envA = 1.0f;
    float envR = 1.0f;
    if (aN > 0 && i < aN) {
      envA = static_cast<float>(static_cast<double>(i) / static_cast<double>(aN));
    }
    if (rN > 0 && i >= n - rN) {
      const size_t k = i - (n - rN);
      envR = static_cast<float>(1.0 - static_cast<double>(k) / static_cast<double>(rN));
    }
    const float env = (envA < envR) ? envA : envR;
    b.data[i] *= env;
  }
}

class SamplerInstrument final : public IInstrument {
public:
  explicit SamplerInstrument(const model::InstrumentSpec& spec) {
    const std::string samplePath = GetString(spec.params, "sample", "");
    if (samplePath.empty()) {
      throw std::runtime_error("SamplerInstrument: missing required param 'sample'");
    }
    m_rootMidi = PitchToMidi(GetString(spec.params, "root", "C4"));
    m_attack = GetFloat(spec.params, "attack", 0.0);
    m_release = GetFloat(spec.params, "release", 0.0);
    if (m_attack < 0.0 || m_release < 0.0) {
      throw std::runtime_error("SamplerInstrument: attack/release must be >= 0");
    }
    audio::AudioBuffer b = audio::WavReader::ReadWav(samplePath);
    m_samples = std::move(b.data);
    if (m_samples.empty()) {
      throw std::runtime_error("SamplerInstrument: sample is empty: " + samplePath);
    }
    size_t ls = 0, le = 0;
    m_hasLoop = TryParseLoop(spec.params, ls, le);
    if (m_hasLoop) {
      if (ls >= m_samples.size()) {
        throw std::runtime_error("SamplerInstrument: loopStart out of range");
      }
      if (le >= m_samples.size()) {
        throw std::runtime_error("SamplerInstrument: loopEnd out of range");
      }
      if (le < ls) {
        throw std::runtime_error("SamplerInstrument: loopEnd must be >= loopStart");
      }
      if (le == ls) {
        throw std::runtime_error("SamplerInstrument: loop length must be > 0");
      }
      m_loopStart = ls;
      m_loopEnd = le;
    }
  }

  audio::AudioBuffer RenderNote(int midi, double durSec, float velocity01, int SampleRate) override {
    audio::AudioBuffer out;
    if (durSec <= 0.0 || SampleRate <= 0) {
      return out;
    }
    if (velocity01 < 0.0f) {
      velocity01 = 0.0f;
    }
    if (velocity01 > 1.0f) {
      velocity01 = 1.0f;
    }
    const size_t n = static_cast<size_t>(std::ceil(durSec * SampleRate));
    out.data.resize(n, 0.0f);
    const double rootHz = MidiToHz(m_rootMidi);
    const double targetHz = MidiToHz(midi);
    const double ratio = targetHz / rootHz;
    const double srcSr = 44100.0;
    const double step = ratio * (srcSr / static_cast<double>(SampleRate));
    double pos = 0.0;
    for (size_t i = 0; i < n; ++i) {
      float s = sampleAt(pos);
      out.data[i] = s * velocity01;
      pos += step;
    }
    ApplyAttackRelease(out, SampleRate, m_attack, m_release);
    return out;
  }

private:
  float sampleAt(double pos) const {
    const size_t len = m_samples.size();
    if (len == 0) {
      return 0.0f;
    }
    if (!m_hasLoop) {
      if (pos < 0.0) {
        return 0.0f;
      }
      const double maxPos = static_cast<double>(len - 1);
      if (pos >= maxPos) {
        return 0.0f;
      }
      return interpLinearNoLoop(pos);
    }
    if (pos < 0.0) {
      return 0.0f;
    }
    if (pos < static_cast<double>(m_loopStart)) {
      const double maxPos = static_cast<double>(len - 1);
      if (pos >= maxPos) {
        return 0.0f;
      }
      return interpLinearNoLoop(pos);
    }
    const size_t loopLen = (m_loopEnd - m_loopStart + 1);
    const double rel = pos - static_cast<double>(m_loopStart);
    const double wrapped = static_cast<double>(m_loopStart) + std::fmod(rel, static_cast<double>(loopLen));
    return interpLinearLoop(wrapped);
  }

  float interpLinearNoLoop(double pos) const {
    const size_t len = m_samples.size();
    const size_t i0 = static_cast<size_t>(std::floor(pos));
    const double frac = pos - static_cast<double>(i0);
    if (i0 >= len) {
      return 0.0f;
    }
    const size_t i1 = (i0 + 1 < len) ? (i0 + 1) : i0;
    const float a = m_samples[i0];
    const float b = m_samples[i1];
    return static_cast<float>((1.0 - frac) * a + frac * b);
  }

  float interpLinearLoop(double pos) const {
    const size_t i0 = static_cast<size_t>(std::floor(pos));
    const double frac = pos - static_cast<double>(i0);
    size_t idx0 = i0;
    size_t idx1 = i0 + 1;
    if (idx0 > m_loopEnd) {
      idx0 = m_loopStart;
    }
    if (idx1 > m_loopEnd) {
      idx1 = m_loopStart;
    }
    const float a = m_samples[idx0];
    const float b = m_samples[idx1];
    return static_cast<float>((1.0 - frac) * a + frac * b);
  }

private:
  std::vector<float> m_samples;
  int m_rootMidi = 60;
  double m_attack = 0.0;
  double m_release = 0.0;
  bool m_hasLoop = false;
  size_t m_loopStart = 0;
  size_t m_loopEnd = 0;
};

} // namespace instruments

namespace {
struct SamplerInstrumentRegistrar {
  SamplerInstrumentRegistrar() {
    factory::InstrumentFactory::RegisterType(
      "sampler",
      [](const model::InstrumentSpec& spec) {
        return std::make_unique<instruments::SamplerInstrument>(spec);
      }
    );
  }
};

static SamplerInstrumentRegistrar g_sampler_registrar;
} // namespace
