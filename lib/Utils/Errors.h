#pragma once

#include <stdexcept>
#include <string>

namespace utils {

class Error : public std::runtime_error {
public:
  explicit Error(const std::string& msg) : std::runtime_error(msg) {}
};

class ParseError : public Error {
public:
  explicit ParseError(const std::string& msg) : Error("ParseError: " + msg) {}
};

class AudioError : public Error {
public:
  explicit AudioError(const std::string& msg) : Error("AudioError: " + msg) {}
};

class ConfigError : public Error {
public:
  explicit ConfigError(const std::string& msg) : Error("ConfigError: " + msg) {}
};

} // namespace utils
