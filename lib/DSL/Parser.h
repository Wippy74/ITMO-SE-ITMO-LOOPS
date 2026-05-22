#pragma once

#include <stdexcept>
#include <string>

#include "Tokenizer.h"
#include "../Model/Composition.h"
#include "../Model/Commands.h"
#include "../Model/InstrumentSpec.h"
#include "../Model/Pattern.h"

namespace dsl {

class ParserError : public std::runtime_error {
public:
  explicit ParserError(const std::string& msg) : std::runtime_error(msg) {}
}; 

class Parser {
public:
  explicit Parser(Tokenizer& tokenizer);
  model::Composition parse();
private:
  void parseBpm(model::Composition& c);
  void ParseInstrumentBlock(model::Composition& c);
  void ParsePatternBlock(model::Composition& c);
  int ReadInt(const Token& t, const std::string& what);
  int ReadIntNext(const std::string& what);
  std::pair<std::string, std::string> readKeyValue();
  static bool IsKeyValueToken(const std::string& s);
  static bool IsCallToken(const std::string& s);
  static std::string StripCallPrefix(const std::string& s);
  void FailHere(const Token& t, const std::string& msg);
  void FailHere(const std::string& msg);
private:
  Tokenizer& tz;
};

} // namespace dsl
