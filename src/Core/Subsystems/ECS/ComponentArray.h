#pragma once

#include "./../../types.h"

#include <cassert>
#include <limits>
#include <unordered_map>

class IComponentArray
{
 public:
     virtual ~IComponentArray() = default;
     virtual void entityDestroyed(Entity entity) = 0;
};

template<typename T>
class ComponentArray : public IComponentArray
{
 public:
    // Link entity and component
    void insert(Entity entity, T component)
    {
        assert(_entityToIndexMap[entity] == std::numeric_limits<Entity>::max() && "Component added to same entity more than once.");

        uint64_t newIndex = _size;
        _entityToIndexMap[entity] = newIndex;
        _indexToEntityMap[newIndex] = entity;
        _componentArray[newIndex] = component;
        ++_size;
    }

    // Remove entity and maintain array density
    void remove(Entity entity)
    {
        assert(_entityToIndexMap[entity] != std::numeric_limits<Entity>::max() && "Trying to remove non-existent component.");

        // Copy element at end into deleted element's index
        // to maintain density
        uint64_t indexRemovedEntity = _entityToIndexMap[entity];
        uint64_t indexLastElement = _size - 1;
        _componentArray[indexRemovedEntity] = _componentArray[indexLastElement];

        // Update map to point to moved spot
        Entity entityLastElement = _indexToEntityMap[indexLastElement];
        _entityToIndexMap[entityLastElement] = indexRemovedEntity;
        _indexToEntityMap[indexRemovedEntity] = entityLastElement;

        _entityToIndexMap[entity] = std::numeric_limits<Entity>::max();
        _indexToEntityMap[indexLastElement] = std::numeric_limits<Entity>::max();
        --_size;
    }

    // Return reference to entity's component
    T& get(Entity entity)
    {
        assert(_entityToIndexMap[entity] != std::numeric_limits<Entity>::max() && "Trying to remove non-existent component.");

        return _componentArray[_entityToIndexMap[entity]];
    }

    // Remove the entity's if it existed
    void entityDestroyed(Entity entity) override
    {
        if (_entityToIndexMap[entity] != std::numeric_limits<Entity>::max())
        {
            remove(entity);
        }
    }

 private:
    std::array<T, MAX_ENTITIES> _componentArray;

    // Fill the array with invalid values
    constexpr std::array<Entity, MAX_ENTITIES> initArrays()
    {
        std::array<Entity, MAX_ENTITIES> temp;
        temp.fill(std::numeric_limits<Entity>::max());
        return temp;
    }

    std::array<Entity, MAX_ENTITIES> _entityToIndexMap = initArrays();
    std::array<Entity, MAX_ENTITIES> _indexToEntityMap = initArrays();
    uint64_t _size = 0;
};
