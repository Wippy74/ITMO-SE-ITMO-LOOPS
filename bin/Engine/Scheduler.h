#pragma once

#include <string>
#include <vector>

#include "../Model/Composition.h"
#include "../Model/Events.h"

namespace engine {

struct SchedulerOptions {
  std::string EntryPattern = "main";
  bool StrictReferences = true;
};

class Scheduler {
public:
  explicit Scheduler(const model::Composition& comp);
  model::NoteEvents schedule(const SchedulerOptions& opt = {});
private:
  void ExpandPattern(const std::string& patternName, double baseSec, std::vector<std::string>& callStack, model::NoteEvents& out);
  const model::Composition& m_comp;
};

} // namespace engine
