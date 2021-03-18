#pragma once

#include <optional>
#include <string_view>

std::optional<int> GetLayerIDByOldName(const std::string_view& layerName);
