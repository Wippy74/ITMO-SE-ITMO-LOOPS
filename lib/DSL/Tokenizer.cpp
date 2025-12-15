#include "Tokenizer.h"

#include <cctype>
#include <fstream>
#include <sstream>

namespace dsl {

static std::string makeLocMsg(const Token& t, const std::string& msg) {
  std::ostringstream oss;
  oss << "Tokenizer error at " << t.line << ":" << t.col << ": " << msg;
  return oss.str();
}

static std::string makeLocMsg(int line, int col, const std::string& msg) {
  std::ostringstream oss;
  oss << "Tokenizer error at " << line << ":" << col << ": " << msg;
  return oss.str();
}

Tokenizer::Tokenizer(std::istream& in) : m_in(&in) {}

Tokenizer::Tokenizer(const std::string& filePath) {
  auto f = std::make_unique<std::ifstream>(filePath);
  if (!f->is_open()) {
    throw TokenizerError("Cannot open file: " + filePath);
  }
  m_in = f.get();
  m_owned = std::move(f);
}

bool Tokenizer::eof() {
  fillBuffer();
  return m_buf.empty() && m_reachedEof;
}

const Token& Tokenizer::peek() {
  fillBuffer();
  if (m_buf.empty()) {
    throw TokenizerError("Unexpected EOF");
  }
  return m_buf.front();
}

Token Tokenizer::next() {
  fillBuffer();
  if (m_buf.empty()) {
    throw TokenizerError("Unexpected EOF");
  }
  Token t = m_buf.front();
  m_buf.pop_front();
  return t;
}

bool Tokenizer::consumeIf(const std::string& expected) {
  fillBuffer();
  if (m_buf.empty()) return false;
  if (m_buf.front().text == expected) {
    m_buf.pop_front();
    return true;
  }
  return false;
}

Token Tokenizer::expect(const std::string& expected) {
  Token t = next();
  if (t.text != expected) {
    throw TokenizerError(makeLocMsg(t, "Expected '" + expected + "', got '" + t.text + "'"));
  }
  return t;
}

void Tokenizer::fillBuffer() {
  if (m_reachedEof) {
    return;
  }
  if (!m_buf.empty()) {
    return;
  }
  std::string line;
  while (std::getline(*m_in, line)) {
    ++m_lineNo;
    tokenizeLine(line);
    if (!m_buf.empty()) {
      return;
    }
  }
  m_reachedEof = true;
}

void Tokenizer::tokenizeLine(const std::string& lineRaw) {
  std::string line = lineRaw;
  for (size_t pos = 0; pos < line.size(); ++pos) {
    if (line[pos] == '#') {
      if (pos == 0 || std::isspace(static_cast<unsigned char>(line[pos - 1]))) {
        line = line.substr(0, pos);
        break;
      }
    }
  }
  const int n = static_cast<int>(line.size());
  int i = 0;
  auto isSpace = [](unsigned char c) -> bool {
    return std::isspace(c) != 0;
  };
  while (i < n) {
    while (i < n && isSpace(static_cast<unsigned char>(line[i]))) {
      ++i;
    }
    if (i >= n) {
      break;
    }
    int start = i;
    while (i < n && !isSpace(static_cast<unsigned char>(line[i]))) {
      ++i;
    }
    int end = i;
    Token t;
    t.text = line.substr(static_cast<size_t>(start), static_cast<size_t>(end - start));
    t.line = m_lineNo;
    t.col  = start + 1;
    if (!t.text.empty()) {
      m_buf.push_back(std::move(t));
    }
  }
}

} // namespace dsl
