#include "resource_locator.hpp"

#include <spdlog/spdlog.h>

#include <string>

namespace storm {

namespace {

constexpr std::string_view ROOT_TEXTURES_DIRECTORY = "RESOURCE/Textures";

} // namespace

std::optional<std::filesystem::path> ResourceLocator::findTexture(const std::string_view &resource_path)
{
    using std::filesystem::path;

    const auto root = path(ROOT_TEXTURES_DIRECTORY, path::format::generic_format);

    std::string_view fixed_resource_path = resource_path;
    if (resource_path.starts_with('\\') || resource_path.starts_with('/')) {
        fixed_resource_path = resource_path.substr(1);
    }

    const std::filesystem::path final_path = root / fixed_resource_path;
    if(std::filesystem::exists(final_path)) {
        return final_path;
    }

    if (m_EnableFileSearch) {
        const path& filename = final_path.filename();
        auto result = findResource(root, filename.string());
        if (result) {
            spdlog::warn("Failed to find file '{}', using a file with the same name found in '{}' instead.", resource_path, result->make_preferred().string());
            return result;
        }
    }

    return {};
}

std::optional<std::filesystem::path> ResourceLocator::findResource(const std::filesystem::path &root,
                                                                   const std::string_view &file_name)
{
    using std::filesystem::path;

    for(auto& directory: std::filesystem::recursive_directory_iterator(root)) {
        const path directory_path = directory.path();
        if (std::filesystem::is_directory(directory_path)) {
            const std::filesystem::path test_path = directory_path / file_name;
            if(std::filesystem::exists(test_path)) {
                return test_path;
            }
        }
    }
    return {};
}

} // namespace storm

