#include "App.h"
#include "Config.h"
#include "DSL/Tokenizer.h"
#include "DSL/Parser.h"
#include "Engine/Scheduler.h"
#include "Engine/Renderer.h"
#include "Audio/WavIO.h"

namespace itmoloops {

int App::run(const std::string& scorePath, const std::string& outWavPath) const {
  dsl::Tokenizer tz(scorePath);
  dsl::Parser parser(tz);
  model::Composition comp = parser.parse();
  engine::Scheduler scheduler(comp);
  engine::SchedulerOptions sopt;
  sopt.EntryPattern = Config::kEntryPattern;
  sopt.StrictReferences = true;
  model::NoteEvents events = scheduler.schedule(sopt);
  engine::RendererOptions ropt;
  ropt.SampleRate = Config::kSampleRate;
  ropt.tailSec = Config::kTailSec;
  ropt.autoNormalize = Config::kAutoNormalize;
  engine::Renderer renderer(comp, ropt);
  audio::AudioBuffer master = renderer.render(events);
  audio::WavWriter::WriteWav(outWavPath, master);
  return 0;
}

} // namespace itmoloops
