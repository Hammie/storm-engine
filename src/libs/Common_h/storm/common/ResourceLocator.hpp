#pragma once

#include <filesystem>
#include <optional>

namespace storm {

class ResourceLocator final
{
  public:
    ResourceLocator() noexcept = default;

    explicit ResourceLocator(bool enable_file_search) noexcept :
        m_EnableFileSearch(enable_file_search)
    {}

    std::optional<std::filesystem::path> findScript(const std::string_view &resource_path);

    std::optional<std::filesystem::path> findTexture(const std::string_view &resource_path);

  private:
    std::optional<std::filesystem::path> findResource(const std::filesystem::path& root, const std::string_view& file_name);

    bool m_EnableFileSearch = false;
};

} // namespace storm
