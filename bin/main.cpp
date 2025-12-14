#include <exception>
#include <iostream>
#include <string>

#include "App.h"

static void printUsage(const char* argv0) {
  std::cerr << "Usage:\n" << "  " << argv0 << " <score.txt> <music.wav>\n";
}

int main(int argc, char** argv) {
  if (argc != 3) {
    printUsage(argv[0]);
    return 1;
  }
  const std::string scorePath = argv[1];
  const std::string outWavPath = argv[2];
  try {
    itmoloops::App app;
    return app.run(scorePath, outWavPath);
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 2;
  } catch (...) {
    std::cerr << "Unknown error\n";
    return 3;
  }
}
