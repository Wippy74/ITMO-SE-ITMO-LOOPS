#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

namespace utils {

using Params = std::unordered_map<std::string, std::string>;

class ParamReader {
public:
  explicit ParamReader(const Params& p);
  bool has(const std::string& key) const;
  int GetInt(const std::string& key) const;
  double GetDouble(const std::string& key) const;
  std::string GetString(const std::string& key) const;
  int GetInt(const std::string& key, int def) const;
  double GetDouble(const std::string& key, double def) const;
  std::string GetString(const std::string& key, const std::string& def) const;
  std::pair<int, int> GetIntPairComma(const std::string& key) const;
  std::optional<std::pair<int, int>> TryGetIntPairComma(const std::string& key) const;

private:
  const Params& m_p;
  static int ParseIntStrict(const std::string& s, const std::string& where);
  static double ParseDoubleStrict(const std::string& s, const std::string& where);
  static std::string trim(const std::string& s);
};

} // namespace utils
