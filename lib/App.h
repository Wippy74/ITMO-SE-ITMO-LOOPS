#pragma once

#include <string>

namespace itmoloops {

class App {
public:
  int run(const std::string& scorePath, const std::string& outWavPath) const;
};

} // namespace itmoloops
