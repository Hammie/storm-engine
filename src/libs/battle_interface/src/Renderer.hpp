#pragma once

#include <storm/common/Handles.hpp>

#include <memory>
#include <vector>

namespace storm::bi
{

class Component
{
  public:
    virtual ~Component() noexcept = default;
};

class Object
{
  public:
    virtual ~Object() noexcept = default;

    // Future TODO: Ranges would work great for this
    template <typename T> std::vector<std::shared_ptr<T>> GetComponents()
    {
        std::vector<std::shared_ptr<T>> results;
        GetComponents<T>(results);
        return results;
    }

    template <typename T> void GetComponents(std::vector<std::shared_ptr<T>> &results)
    {
        for (const auto &component : components_)
        {
            auto component_ptr = std::dynamic_pointer_cast<T>(component);
            if (component_ptr != nullptr)
            {
                results.emplace_back(std::move(component_ptr));
            }
        }
    }

    Object &AddComponent(std::shared_ptr<Component> component)
    {
        components_.emplace_back(std::move(component));
        return *this;
    }

  private:
    std::vector<std::shared_ptr<Component>> components_;
};

class Renderer
{
  public:
    void Draw();

  private:
    std::vector<std::shared_ptr<Object>> objects_;
};

class Sprite final : public Component
{
  public:
    ~Sprite() noexcept override = default;
};

inline void Renderer::Draw()
{
    std::vector<std::shared_ptr<Sprite>> sprites{};

    for (const auto &object : objects_)
    {
        object->GetComponents<Sprite>(sprites);
    }

    // TODO: Sort sprites by layer and material
}

// -----------------------------------------------------------------------------

class ManSign final : public Object
{
  public:
    ~ManSign() noexcept override = default;

    // Sprites:
    // * Face
    // * Back
    // * Alarm icon
    // * State (HP & Energy)
    // * Gun charge
};

// objects[0].components[0] = new BISprite(0xFF9900, myTexture, position, size);

} // namespace storm::bi
