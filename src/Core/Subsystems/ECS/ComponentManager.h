#pragma once

#include "../../types.h"
#include "ComponentArray.h"

#include <typeinfo>
#include <unordered_map>
#include <memory>

class ComponentManager
{
 public:
    template<typename T>
    void RegisterComponent()
    {
        const char* typeName = typeid(T).name();

        assert(_componentTypes.find(typeName) == _componentTypes.end()
                && "Registering component type more than once.");
        
        // Add the component type to the component type map
        _componentTypes.insert({typeName, _nextComponentType});

        // Create a ComponentArray pointer and add it to the component array
        // map
        _componentArrays.insert(
                {typeName, std::make_shared<ComponentArray<T>>()}
        );

        // Increment the next component type id
        ++_nextComponentType;
    }

    template<typename T>
    ComponentType GetComponentType()
    {
        const char* typeName = typeid(T).name();

        assert(_componentTypes.find(typeName) != _componentTypes.end()
                && "Component not registered before use.");

        // Return this component's type
        return _componentTypes[typeName];
    }

    // Add a component to the array for an entity
    template<typename T>
    void AddComponent(Entity entity, T component)
    {
        GetComponentArray<T>()->Insert(entity, component); 
    }

    // Remove a component from the array for an entity
    template<typename T>
    void RemoveComponent(Entity entity, T component)
    {
        GetComponentArray<T>()->Remove(entity, component); 
    }

    // Get a reference to a component from the array for an entity
    template<typename T>
    T& GetComponent(Entity entity)
    {
        return GetComponentArray<T>()->Get(entity);
    }

    // Notify each component array that an entity has been destroyed
    // If it has a component for that entity, it will remove it
    void EntityDestroyed(Entity entity)
    {
        for (const auto& pair : _componentArrays)
        {
            const auto& component = pair.second;
            component->EntityDestroyed(entity);
        }
    }

 private:
    // Map from type string pointer to a component type
    std::unordered_map<const char*, ComponentType> _componentTypes{};

    // Map from type string pointer to a component array
    std::unordered_map<const char*,
        std::shared_ptr<IComponentArray>> _componentArrays{};

    // The next component type starting at 0
    ComponentType _nextComponentType{};

    // Get statically casted pointer to the ComponentArray of type T
    template<typename T>
    std::shared_ptr<ComponentArray<T>> GetComponentArray()
    {
        const char* typeName = typeid(T).name();
        assert(_componentTypes.find(typeName) != _componentTypes.end()
                && "Component not registered before use.");
        return std::static_pointer_cast<ComponentArray<T>>
            (_componentArrays[typeName]);
    }
};
