#pragma once

#include "../Model/Composition.h"
#include "../Model/Events.h"
#include "../Audio/AudioBuffer.h"

namespace engine {

struct RendererOptions {
  int SampleRate = 44100;
  double tailSec = 2.0;
  bool autoNormalize = true;
};

class Renderer {
public:
  explicit Renderer(const model::Composition& comp, RendererOptions opt = {});
  audio::AudioBuffer render(const model::NoteEvents& events);
private:
  const model::Composition& m_comp;
  RendererOptions m_opt;
};

} // namespace engine
