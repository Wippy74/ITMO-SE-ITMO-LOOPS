#include "Parser.h"

#include <sstream>
#include <unordered_set>

namespace dsl {

static std::string loc(const Token& t) {
  std::ostringstream oss;
  oss << t.line << ":" << t.col;
  return oss.str();
}

Parser::Parser(Tokenizer& tokenizer) : tz(tokenizer) {}

model::Composition Parser::parse() {
  model::Composition c;
  bool bpmSeen = false;
  while (!tz.eof()) {
    const Token& t = tz.peek();
    if (t.text == "bpm") {
      if (bpmSeen) {
        FailHere(t, "Duplicate 'bpm' directive");
      }
      parseBpm(c);
      bpmSeen = true;
    } else if (t.text == "instrument") {
      ParseInstrumentBlock(c);
    } else if (t.text == "pattern") {
      ParsePatternBlock(c);
    } else {
      FailHere(t, "Unknown top-level directive '" + t.text + "'");
    }
  }
  if (!bpmSeen) {
    FailHere("Missing required 'bpm' directive");
  }
  if (c.patterns.find("main") == c.patterns.end()) {
    throw ParserError("Parser error: missing required pattern 'main'");
  }
  return c;
}

void Parser::parseBpm(model::Composition& c) {
  Token kw = tz.next();
  (void)kw;
  Token v = tz.next();
  int bpm = ReadInt(v, "bpm");
  if (bpm <= 0) {
    FailHere(v, "bpm must be > 0");
  }
  c.bpm = bpm;
}

void Parser::ParseInstrumentBlock(model::Composition& c) {
  Token kw = tz.next();
  (void)kw;
  Token nameTok = tz.next();
  Token typeTok = tz.next();
  const std::string instName = nameTok.text;
  const std::string instType = typeTok.text;
  if (c.instruments.find(instName) != c.instruments.end()) {
    FailHere(nameTok, "Duplicate instrument name '" + instName + "'");
  }
  model::InstrumentSpec spec;
  spec.name = instName;
  spec.type = instType;
  bool ended = false;
  while (!tz.eof()) {
    const Token& t = tz.peek();
    if (t.text == "end") {
      tz.next();
      ended = true;
      break;
    }
    if (t.text == "effect") {
      tz.next();
      Token effTypeTok = tz.next();
      model::EffectSpec eff;
      eff.type = effTypeTok.text;
      while (!tz.eof()) {
        const Token& p = tz.peek();
        if (p.text == "effect" || p.text == "end") break;
        auto [k, v] = readKeyValue();
        if (eff.params.find(k) != eff.params.end()) {
          FailHere(p, "Duplicate effect param '" + k + "' for effect '" + eff.type + "'");
        }
        eff.params[k] = v;
      }
      spec.effects.push_back(std::move(eff));
      continue;
    }
    auto [k, v] = readKeyValue();
    if (spec.params.find(k) != spec.params.end()) {
      FailHere(t, "Duplicate instrument param '" + k + "' for instrument '" + instName + "'");
    }
    spec.params[k] = v;
  }
  if (!ended) {
    throw ParserError("Parser error: unexpected EOF inside instrument '" + instName + "' block");
  }
  c.instruments.emplace(instName, std::move(spec));
}


void Parser::ParsePatternBlock(model::Composition& c) {
  Token kw = tz.next();
  (void)kw;
  Token nameTok = tz.next();
  std::string patName = nameTok.text;
  if (c.patterns.find(patName) != c.patterns.end()) {
    FailHere(nameTok, "Duplicate pattern name '" + patName + "'");
  }
  int resolution = 0;
  if (tz.eof()) FailHere("Unexpected EOF after pattern name");
  const Token& nextTok = tz.peek();
  if (nextTok.text == "resolution") {
    tz.next();
    resolution = ReadIntNext("pattern resolution");
  } else {
    Token rTok = tz.next();
    resolution = ReadInt(rTok, "pattern resolution");
  }
  if (resolution <= 0) FailHere(nameTok, "pattern resolution must be > 0");
  model::Pattern p;
  p.name = patName;
  p.resolution = resolution;
  bool ended = false;
  while (!tz.eof()) {
    const Token& t = tz.peek();
    if (t.text == "end") {
      tz.next();
      ended = true;
      break;
    }
    Token startTok = tz.next();
    int startUnits = ReadInt(startTok, "note start (units)");
    if (startUnits < 0) {
      FailHere(startTok, "start must be >= 0");
    }
    if (tz.eof()) {
      FailHere("Unexpected EOF inside pattern '" + patName + "'");
    }
    Token second = tz.next();
    if (IsCallToken(second.text)) {
      model::CallCmd cmd;
      cmd.startUnits = startUnits;
      cmd.patternName = StripCallPrefix(second.text);
      if (cmd.patternName.empty()) {
        FailHere(second, "Empty pattern call token");
      }
      p.commands.emplace_back(std::move(cmd));
      continue;
    }
    model::NoteCmd n;
    n.startUnits = startUnits;
    n.instrument = second.text;
    if (tz.eof()) {
      FailHere("Unexpected EOF (pitch) in pattern '" + patName + "'");
    }
    Token pitchTok = tz.next();
    n.pitch = pitchTok.text;
    if (tz.eof()) {
      FailHere("Unexpected EOF (duration) in pattern '" + patName + "'");
    }
    Token durTok = tz.next();
    n.durUnits = ReadInt(durTok, "note duration (units)");
    if (n.durUnits < 0) {
      FailHere(durTok, "duration must be > 0");
    }
    if (tz.eof()) {
      FailHere("Unexpected EOF (velocity) in pattern '" + patName + "'");
    }
    Token velTok = tz.next();
    n.velocity = ReadInt(velTok, "note velocity");
    if (n.velocity < 0 || n.velocity > 100) {
      FailHere(velTok, "velocity must be in [0..100]");
    }
    if (n.durUnits == 0) {
      continue;
    }
    p.commands.emplace_back(std::move(n));
  }
  if (!ended) {
    throw ParserError("Parser error: unexpected EOF inside pattern '" + patName + "' block");
  }
  c.patterns.emplace(patName, std::move(p));
}


int Parser::ReadInt(const Token& t, const std::string& what) {
  try {
    size_t idx = 0;
    int v = std::stoi(t.text, &idx, 10);
    if (idx != t.text.size()) {
      FailHere(t, "Invalid integer for " + what + ": '" + t.text + "'");
    }
    return v;
  } catch (...) {
    FailHere(t, "Invalid integer for " + what + ": '" + t.text + "'");
  }
}

int Parser::ReadIntNext(const std::string& what) {
  Token t = tz.next();
  return ReadInt(t, what);
}

bool Parser::IsKeyValueToken(const std::string& s) {
  return s.find('=') != std::string::npos;
}

std::pair<std::string, std::string> Parser::readKeyValue() {
  if (tz.eof()) {
    FailHere("Unexpected EOF while reading key/value");
  }
  Token first = tz.next();
  auto pos = first.text.find('=');
  if (pos != std::string::npos) {
    std::string k = first.text.substr(0, pos);
    std::string v = first.text.substr(pos + 1);
    if (k.empty() || v.empty()) {
      FailHere(first, "Invalid key=value token: '" + first.text + "'");
    }
    return {k, v};
  }
  if (tz.eof()) {
    FailHere(first, "Unexpected EOF after key '" + first.text + "'");
  }
  Token eq = tz.next();
  if (eq.text != "=") {
    FailHere(eq, "Expected '=' after key '" + first.text + "', got '" + eq.text + "'");
  }
  if (tz.eof()) FailHere(eq, "Unexpected EOF after '='");
  Token val = tz.next();
  if (first.text.empty() || val.text.empty()) {
    FailHere(first, "Empty key or value in key = value");
  }
  return {first.text, val.text};
}

bool Parser::IsCallToken(const std::string& s) {
  return !s.empty() && s[0] == '@';
}

std::string Parser::StripCallPrefix(const std::string& s) {
  if (s.empty() || s[0] != '@') return s;
  return s.substr(1);
}

void Parser::FailHere(const Token& t, const std::string& msg) {
  throw ParserError("Parser error at " + loc(t) + ": " + msg);
}

void Parser::FailHere(const std::string& msg) {
  throw ParserError("Parser error: " + msg);
}

} // namespace dsl
