set(CMAKE_CXX_STANDARD 20)

add_library(itmoloops_lib
  App.cpp
  Audio/AudioBuffer.cpp
  Audio/WavReader.cpp
  Audio/WavWritter.cpp
  DSL/Parser.cpp
  DSL/Tokenizer.cpp
  Effects/EchoEffect.cpp
  Effects/GainEffect.cpp
  Effects/TremoloEffect.cpp
  Engine/Renderer.cpp
  Engine/Scheduler.cpp
  Factory/EffectFactory.cpp
  Factory/InstrumentFactory.cpp
  Instruments/SamplerInstrument.cpp
  Instruments/SineInstrument.cpp
  Instruments/SquareInstrument.cpp
  Instruments/TriangleInstrument.cpp
  Utils/Params.cpp
  Utils/Pitch.cpp
)

target_include_directories(itmoloops_lib PUBLIC .)
