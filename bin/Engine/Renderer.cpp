#include "../Engine/Renderer.h"
#include "../Factory/EffectFactory.h"
#include "../Factory/InstrumentFactory.h"
#include "../Effects/IEffect.h"
#include "../Instruments/IInstrument.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>


namespace engine {

static size_t SecToSamples(double sec, int sr) {
  if (sec <= 0.0 || sr <= 0) {
    return 0;
  }
  return static_cast<size_t>(std::ceil(sec * static_cast<double>(sr)));
}

static float PeakAbs(const audio::AudioBuffer& b) {
  float p = 0.0f;
  for (float x : b.data) {
    const float a = std::fabs(x);
    if (a > p) {
      p = a;
    }
  }
  return p;
}

Renderer::Renderer(const model::Composition& comp, RendererOptions opt)
  : m_comp(comp), m_opt(opt) {
  if (m_opt.SampleRate <= 0) {
    throw std::runtime_error("Renderer: SampleRate must be > 0");
  }
}

audio::AudioBuffer Renderer::render(const model::NoteEvents& events) {
  audio::AudioBuffer master;
  if (events.empty()) {
    return master;
  }
  std::unordered_map<std::string, std::vector<const model::NoteEvent*>> byInst;
  byInst.reserve(events.size());
  double lastEnd = 0.0;
  for (const auto& e : events) {
    byInst[e.instrument].push_back(&e);
    lastEnd = std::max(lastEnd, e.startSec + e.durSec);
  }
  const size_t baseSamples = SecToSamples(lastEnd + m_opt.tailSec, m_opt.SampleRate);
  master.resize(baseSamples, 0.0f);
  for (const auto& [instName, evPtrs] : byInst) {
    auto itSpec = m_comp.instruments.find(instName);
    if (itSpec == m_comp.instruments.end()) {
      throw std::runtime_error("Renderer: instrument '" + instName + "' not declared");
    }
    const model::InstrumentSpec& spec = itSpec->second;
    std::unique_ptr<instruments::IInstrument> inst = factory::InstrumentFactory::create(spec);
    audio::AudioBuffer track;
    track.resize(baseSamples, 0.0f);
    for (const model::NoteEvent* pe : evPtrs) {
      const model::NoteEvent& e = *pe;
      const size_t offset = SecToSamples(e.startSec, m_opt.SampleRate);
      audio::AudioBuffer note = inst->RenderNote(e.midi, e.durSec, e.velocity01, m_opt.SampleRate);
      track.MixAddFrom(note, offset);
    }
    for (const auto& effSpec : spec.effects) {
      std::unique_ptr<effects::IEffect> eff = factory::EffectFactory::create(effSpec);
      eff->process(track, m_opt.SampleRate);
    }
    master.MixAddFrom(track, 0);
  }
  if (m_opt.autoNormalize) {
    const float p = PeakAbs(master);
    if (p > 1.0f) {
      master.ApplyGain(1.0f / p);
    }
  }
  master.clip(-1.0f, 1.0f);
  return master;
}

} // namespace engine
