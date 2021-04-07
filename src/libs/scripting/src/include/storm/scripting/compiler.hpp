#ifndef STORM_ENGINE_COMPILER_HPP
#define STORM_ENGINE_COMPILER_HPP
#pragma once

#include <cstdint>
#include <limits>
#include <string_view>

namespace storm::scripting {

constexpr uint32_t INVALID_SEGMENT_INDEX = std::numeric_limits<uint32_t>::max();

enum class COMPILER_STAGE
{
    CS_SYSTEM,
    CS_COMPILATION,
    CS_RUNTIME
};

class Compiler {
  public:
    static size_t getNextTokenLength(const std::string_view &input, bool keep_control_symbols = false);
};

} // namespace storm::scripting

#endif // STORM_ENGINE_COMPILER_HPP
