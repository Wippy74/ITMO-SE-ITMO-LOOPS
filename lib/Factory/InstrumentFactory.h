#pragma once

#include <functional>
#include <memory>
#include <string>

#include "../Instruments/IInstrument.h"
#include "../Model/InstrumentSpec.h"

namespace factory {

class InstrumentFactory {
public:
  using Creator = std::function<std::unique_ptr<instruments::IInstrument>(const model::InstrumentSpec&)>;
  static bool RegisterType(const std::string& type, Creator creator);
  static std::unique_ptr<instruments::IInstrument> create(const model::InstrumentSpec& spec);
  static bool IsRegistered(const std::string& type);
  static void clear();

private:
  InstrumentFactory() = default;
};

} // namespace factory
