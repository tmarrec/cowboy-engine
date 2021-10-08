#pragma once

#include "../../types.h"

#include <queue>
#include <array>
#include <cassert>

class EntityManager
{
 public:
     EntityManager()
     {
         // Fill the queue with all the possible entity IDs
         for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
         {
            _availableEntities.push(entity);
         }
     }

     // Create a new entity
     Entity createEntity()
     {
        assert(_availableEntities.size() > 0 && "Reached entities limit.");

        // Take an ID from unused entities queue
        Entity id = _availableEntities.front();
        _availableEntities.pop();

        return id;
     }

     // Destroy an entity
     void destroyEntity(Entity entity)
     {
        assert(entity < MAX_ENTITIES && "Entity out of range.");

        // Invalidate the destroyed entity's signature
        _signatures[entity].reset();

        // Put the destroyed entity's ID back to the unused entities queue
        _availableEntities.push(entity);
     }

     // Set signature of an entity
     Signature& sig(Entity entity)
     {
        assert(entity < MAX_ENTITIES && "Entity out of range.");

        return _signatures[entity];
     }

     // Get signature of an entity
     Signature sig(Entity entity) const
     {
        assert(entity < MAX_ENTITIES && "Entity out of range.");

        return _signatures[entity];
     }

 private:
     // Queue of unused entity IDs
     std::queue<Entity> _availableEntities {};

     // Array of signatures where the index corresponds to the entity ID
     std::array<Signature, MAX_ENTITIES> _signatures {};
};
