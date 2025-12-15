#include "Params.h"

#include <cctype>
#include <stdexcept>
#include <string>
#include <optional>
#include <utility>
#include <cstdint>

#include "Errors.h"

namespace utils {

ParamReader::ParamReader(const Params& p) : m_p(p) {}

bool ParamReader::Has(const std::string& key) const {
  return m_p.find(key) != m_p.end();
}

int ParamReader::GetInt(const std::string& key) const {
  auto it = m_p.find(key);
  if (it == m_p.end()) {
    throw ConfigError("Missing required int param '" + key + "'");
  }
  return ParseIntStrict(it->second, "param '" + key + "'");
}

double ParamReader::GetDouble(const std::string& key) const {
  auto it = m_p.find(key);
  if (it == m_p.end()) {
    throw ConfigError("Missing required double param '" + key + "'");
  }
  return ParseDoubleStrict(it->second, "param '" + key + "'");
}

std::string ParamReader::GetString(const std::string& key) const {
  auto it = m_p.find(key);
  if (it == m_p.end()) {
    throw ConfigError("Missing required string param '" + key + "'");
  }
  return it->second;
}

int ParamReader::GetInt(const std::string& key, int def) const {
  auto it = m_p.find(key);
  if (it == m_p.end()) return def;
  return ParseIntStrict(it->second, "param '" + key + "'");
}

double ParamReader::GetDouble(const std::string& key, double def) const {
  auto it = m_p.find(key);
  if (it == m_p.end()) return def;
  return ParseDoubleStrict(it->second, "param '" + key + "'");
}

std::string ParamReader::GetString(const std::string& key, const std::string& def) const {
  auto it = m_p.find(key);
  if (it == m_p.end()) return def;
  return it->second;
}

std::pair<int, int> ParamReader::GetIntPairComma(const std::string& key) const {
  auto it = m_p.find(key);
  if (it == m_p.end()) {
    throw ConfigError("Missing required pair param '" + key + "'");
  }

  const std::string s = it->second;
  const auto pos = s.find(',');
  if (pos == std::string::npos) {
    throw ConfigError("Param '" + key + "' must be in format 'a,b'");
  }

  const std::string a = trim(s.substr(0, pos));
  const std::string b = trim(s.substr(pos + 1));

  int ia = ParseIntStrict(a, "param '" + key + "' (left)");
  int ib = ParseIntStrict(b, "param '" + key + "' (right)");
  return {ia, ib};
}

std::optional<std::pair<int, int>> ParamReader::TryGetIntPairComma(const std::string& key) const {
  auto it = m_p.find(key);
  if (it == m_p.end()) return std::nullopt;

  return GetIntPairComma(key);
}

int ParamReader::ParseIntStrict(const std::string& s, const std::string& where) {
  try {
    size_t idx = 0;
    int v = std::stoi(trim(s), &idx, 10);
    const std::string ts = trim(s);
    if (idx != ts.size()) {
      throw ConfigError("Invalid int in " + where + ": '" + s + "'");
    }
    return v;
  } catch (const Error&) {
    throw;
  } catch (...) {
    throw ConfigError("Invalid int in " + where + ": '" + s + "'");
  }
}

double ParamReader::ParseDoubleStrict(const std::string& s, const std::string& where) {
  try {
    const std::string ts = trim(s);
    size_t idx = 0;
    double v = std::stod(ts, &idx);
    if (idx != ts.size()) {
      throw ConfigError("Invalid double in " + where + ": '" + s + "'");
    }
    return v;
  } catch (const Error&) {
    throw;
  } catch (...) {
    throw ConfigError("Invalid double in " + where + ": '" + s + "'");
  }
}

std::string ParamReader::trim(const std::string& s) {
  size_t l = 0;
  while (l < s.size() && std::isspace(static_cast<unsigned char>(s[l]))) {
    ++l;
  }
  size_t r = s.size();
  while (r > l && std::isspace(static_cast<unsigned char>(s[r - 1]))) {
    --r;
  }
  return s.substr(l, r - l);
}

} // namespace utils
