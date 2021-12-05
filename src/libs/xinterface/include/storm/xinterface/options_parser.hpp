#pragma once

#include "attributes.h"

#include <string>

namespace storm {

namespace detail {

void removeCarriageReturn(std::string &str);

} // namespace detail

void parseOptions(const std::string_view &str, ATTRIBUTES &attribute);

} // namespace storm
