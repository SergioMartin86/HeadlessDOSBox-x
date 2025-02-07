#pragma once

// Base controller class
// by eien86

#include <cstdint>
#include <jaffarCommon/exceptions.hpp>
#include <jaffarCommon/json.hpp>
#include <string>
#include <sstream>

namespace jaffar
{

typedef uint64_t input_t;

class InputParser
{
public:

  enum controller_t { jaffar };

  InputParser(const nlohmann::json &config)
  {
  }

  inline input_t parseInputString(const std::string &inputString) const
  {
    // Storage for the input
    input_t input;

    // Converting input into a stream for parsing
    std::istringstream ss(inputString);

    // Start separator
    char c = ss.get();
    if (c != '|') reportBadInputString(inputString, c);

    // End separator
    c = ss.get();
    if (c != '|') reportBadInputString(inputString, c);

    // If its not the end of the stream, then extra values remain and its invalid
    c = ss.get();
    if (ss.eof() == false) reportBadInputString(inputString, c);

    // Returning input
    return input;
  };

  private:


  static inline void reportBadInputString(const std::string &inputString, const char c)
  {
    JAFFAR_THROW_LOGIC("Could not decode input string: '%s' - Read: '%c' (%u)\n", inputString.c_str(), c, (uint8_t)c);
  }

  uint8_t _playerCount;
  controller_t _controllerType;
}; // class InputParser

} // namespace jaffar