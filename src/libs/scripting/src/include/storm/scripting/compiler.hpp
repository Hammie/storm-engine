#ifndef STORM_ENGINE_COMPILER_HPP
#define STORM_ENGINE_COMPILER_HPP
#pragma once

#include <cstdint>
#include <limits>

namespace storm::scripting {

constexpr uint32_t INVALID_SEGMENT_INDEX = std::numeric_limits<uint32_t>::max();

enum class COMPILER_STAGE
{
    CS_SYSTEM,
    CS_COMPILATION,
    CS_RUNTIME
};

} // namespace storm::scripting

#endif // STORM_ENGINE_COMPILER_HPP
