#include "compatibility.hpp"

#include "layers.h"

std::optional<int> GetLayerIDByOldName(const std::string_view& layerName) {
    if (layerName == "execute") {
        return EXECUTE;
    } else if (layerName == "realize") {
        return REALIZE;
    } else if (layerName == "iExecute" || layerName == "iexecute") {
        return INTERFACE_EXECUTE;
    } else if (layerName == "iRealize" || layerName == "irealize") {
        return INTERFACE_REALIZE;
    } else if (layerName == "sea_execute") {
        return SEA_EXECUTE;
    } else if (layerName == "sea_realize") {
        return SEA_REALIZE;
    } else if (layerName == "sea_reflection") {
        return SEA_REFLECTION;
    } else if (layerName == "inf_realize") {
        return INFO_REALIZE;
    } else if (layerName == "sails_trace") {
        return SAILS_TRACE;
    } else if (layerName == "system_messages") {
        return INFO_REALIZE; // Not sure about this one
    } else {
        return std::nullopt;
    }
}
