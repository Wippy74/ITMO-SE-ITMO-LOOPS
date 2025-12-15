#include "../Engine/Scheduler.h"
#include "../Model/Commands.h"
#include "../Utils/Pitch.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <stdexcept>
#include <unordered_set>

namespace engine {

static double BeatSec(int bpm) {
  return 60.0 / static_cast<double>(bpm);
}

static std::string JoinStack(const std::vector<std::string>& stack) {
  std::ostringstream oss;
  for (size_t i = 0; i < stack.size(); ++i) {
    if (i) oss << " -> ";
    oss << stack[i];
  }
  return oss.str();
}

Scheduler::Scheduler(const model::Composition& comp) : m_comp(comp) {}

model::NoteEvents Scheduler::schedule(const SchedulerOptions& opt) {
  if (m_comp.bpm <= 0) {
    throw std::runtime_error("Scheduler: bpm must be > 0");
  }
  if (m_comp.patterns.find(opt.EntryPattern) == m_comp.patterns.end()) {
    throw std::runtime_error("Scheduler: entry pattern '" + opt.EntryPattern + "' not found");
  }
  model::NoteEvents out;
  std::vector<std::string> callStack;
  ExpandPattern(opt.EntryPattern, 0.0, callStack, out);
  std::sort(out.begin(), out.end(), [](const model::NoteEvent& a, const model::NoteEvent& b) {
    if (a.startSec != b.startSec) return a.startSec < b.startSec;
    return a.instrument < b.instrument;
  });
  if (opt.StrictReferences) {
    for (const auto& e : out) {
      if (m_comp.instruments.find(e.instrument) == m_comp.instruments.end()) {
        throw std::runtime_error("Scheduler: instrument '" + e.instrument + "' referenced but not declared");
      }
    }
  }

  return out;
}

void Scheduler::ExpandPattern(const std::string& patternName, double baseSec, std::vector<std::string>& callStack, model::NoteEvents& out) {
  for (const auto& s : callStack) {
    if (s == patternName) {
      std::vector<std::string> cycle = callStack;
      cycle.push_back(patternName);
      throw std::runtime_error("Scheduler: pattern call cycle detected: " + JoinStack(cycle));
    }
  }
  auto it = m_comp.patterns.find(patternName);
  if (it == m_comp.patterns.end()) {
    throw std::runtime_error("Scheduler: pattern '" + patternName + "' not found");
  }
  const model::Pattern& p = it->second;
  if (p.resolution <= 0) {
    throw std::runtime_error("Scheduler: pattern '" + patternName + "' has invalid resolution");
  }
  callStack.push_back(patternName);
  const double unitSec = BeatSec(m_comp.bpm) / static_cast<double>(p.resolution);
  for (const auto& cmd : p.commands) {
    if (std::holds_alternative<model::CallCmd>(cmd)) {
      const auto& c = std::get<model::CallCmd>(cmd);
      if (c.startUnits < 0) {
        throw std::runtime_error("Scheduler: negative startUnits in pattern '" + patternName + "'");
      }
      const double childBase = baseSec + static_cast<double>(c.startUnits) * unitSec;
      ExpandPattern(c.patternName, childBase, callStack, out);
      continue;
    }
    const auto& n = std::get<model::NoteCmd>(cmd);
    if (n.startUnits < 0 || n.durUnits < 0) {
      throw std::runtime_error("Scheduler: invalid note timing in pattern '" + patternName + "'");
    }
    if (n.durUnits == 0) {
      continue;
    }
    model::NoteEvent e;
    e.startSec = baseSec + static_cast<double>(n.startUnits) * unitSec;
    e.durSec = static_cast<double>(n.durUnits) * unitSec;
    e.midi = utils::Pitch::ToMidi(n.pitch);
    float v = static_cast<float>(n.velocity) / 100.0f;
    if (v < 0.0f) {
      v = 0.0f;
    }
    if (v > 1.0f) {
      v = 1.0f;
    } 
    e.velocity01 = v;
    e.instrument = n.instrument;
    out.push_back(std::move(e));
  }
  callStack.pop_back();
}

} // namespace engine
