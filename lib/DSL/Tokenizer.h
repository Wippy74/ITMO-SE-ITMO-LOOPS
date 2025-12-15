#pragma once

#include <deque>
#include <istream>
#include <memory>
#include <stdexcept>
#include <string>

namespace dsl {

struct Token {
  std::string text;
  int line = 1;
  int col  = 1;
};

class TokenizerError : public std::runtime_error {
public:
  explicit TokenizerError(const std::string& msg) : std::runtime_error(msg) {}
};

class Tokenizer {
public:
  explicit Tokenizer(std::istream& in);
  explicit Tokenizer(const std::string& filePath);
  bool eof();
  const Token& peek();
  Token next();
  bool consumeIf(const std::string& expected);
  Token expect(const std::string& expected);
  int currentLine() const { return m_lineNo; }
private:
  void fillBuffer();
  void tokenizeLine(const std::string& line);
  std::istream* m_in = nullptr;
  std::unique_ptr<std::istream> m_owned;
  std::deque<Token> m_buf;
  bool m_reachedEof = false;
  int  m_lineNo = 0;
};

} // namespace dsl
