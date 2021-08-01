#include "storm/common/ResourceLocator.hpp"

#include <spdlog/spdlog.h>

#include <string>

namespace storm
{

namespace
{

constexpr std::string_view ROOT_MODS_DIRECTORY = "Mods";
constexpr std::string_view ROOT_PROGRAM_DIRECTORY = "PROGRAM";
constexpr std::string_view ROOT_TEXTURES_DIRECTORY = "RESOURCE/Textures";

} // namespace

std::optional<std::filesystem::path> ResourceLocator::findScript(const std::string_view &resource_path)
{
    using std::filesystem::path;

    const auto root = path(ROOT_PROGRAM_DIRECTORY, path::format::generic_format);

    std::string_view fixed_resource_path = resource_path;
    if (resource_path.starts_with('\\') || resource_path.starts_with('/'))
    {
        fixed_resource_path = resource_path.substr(1);
    }

    const std::filesystem::path final_path = fixed_resource_path;
    const path &filename = final_path.filename();

    // Check mods folders
    auto mod_result = findModResource({}, fixed_resource_path);
    if (mod_result)
    {
        return mod_result;
    }

    if (std::filesystem::exists(final_path))
    {
        return final_path;
    }

    if (m_EnableFileSearch)
    {
        auto result = findResource(root, filename.string());
        if (result)
        {
            spdlog::warn("Failed to find file '{}', using a file with the same name found in '{}' instead.",
                         resource_path, result->make_preferred().string());
            return result;
        }
    }

    return {};
}

std::optional<std::filesystem::path> ResourceLocator::findTexture(const std::string_view &resource_path)
{
    return findTexture(resource_path, ROOT_TEXTURES_DIRECTORY);
}

std::optional<std::filesystem::path> ResourceLocator::findTexture(const std::string_view &resource_path,
                                                                  const std::string_view &root_dir)
{
    using std::filesystem::path;

    const auto root = path(root_dir, path::format::generic_format);

    std::string_view fixed_resource_path = resource_path;
    if (resource_path.starts_with('\\') || resource_path.starts_with('/'))
    {
        fixed_resource_path = resource_path.substr(1);
    }

    const std::filesystem::path final_path = root / fixed_resource_path;
    const path &filename = final_path.filename();

    // Check mods folders
    auto mod_result = findModResource(root, fixed_resource_path);
    if (mod_result)
    {
        return mod_result;
    }

    if (std::filesystem::exists(final_path))
    {
        return final_path;
    }

    if (m_EnableFileSearch)
    {
        auto result = findResource(root, filename.string());
        if (result)
        {
            spdlog::warn("Failed to find file '{}', using a file with the same name found in '{}' instead.",
                         resource_path, result->make_preferred().string());
            return result;
        }
    }

    return {};
}

std::optional<std::filesystem::path> ResourceLocator::findModResource(const std::filesystem::path &root,
                                                                      const std::string_view &file_name)
{
    using std::filesystem::path;

    // Check mods folders
    const auto mods = path(ROOT_MODS_DIRECTORY, path::format::generic_format);
    if (std::filesystem::exists(mods))
    {
        for (auto &directory : std::filesystem::directory_iterator(mods))
        {
            const path &directory_path = directory.path();
            if (std::filesystem::is_directory(directory_path))
            {
                const auto mod_search_directory = root.empty() ? directory_path : directory_path / root;
                const std::filesystem::path test_path = mod_search_directory / file_name;
                if (std::filesystem::exists(test_path))
                {
                    return test_path;
                }
            }
        }
    }

    return {};
}

std::optional<std::filesystem::path> ResourceLocator::findResource(const std::filesystem::path &root,
                                                                   const std::string_view &file_name)
{
    using std::filesystem::path;

    // Check standard folders
    if (std::filesystem::exists(root))
    {
        for (auto &directory : std::filesystem::recursive_directory_iterator(root))
        {
            const path &directory_path = directory.path();
            if (std::filesystem::is_directory(directory_path))
            {
                const std::filesystem::path test_path = directory_path / file_name;
                if (std::filesystem::exists(test_path))
                {
                    return test_path;
                }
            }
        }
    }
    return {};
}

} // namespace storm
