#include <exception>
#include <iostream>
#include <string>

#include "../lib/App.h"

static void PrintUsage(const char* argv0) {
  std::cerr << "Usage:\n" << "  " << argv0 << " <score.txt> <music.wav>\n";
}

int main(int argc, char** argv) {
  if (argc != 3) {
    PrintUsage(argv[0]);
    return 1;
  }
  const std::string ScorePath = argv[1];
  const std::string OutWavPath = argv[2];
  try {
    itmoloops::App app;
    return app.run(ScorePath, OutWavPath);
  } catch (const std::exception& e) {
    std::cerr << e.what() << "\n";
    return 2;
  } catch (...) {
    std::cerr << "Unknown error\n";
    return 3;
  }
}
