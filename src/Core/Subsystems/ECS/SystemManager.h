#pragma once

#include "System.h"
#include "../../types.h"

#include <memory>
#include <unordered_map>
#include <cassert>

class SystemManager
{
 public:
    // Create a pointer to the system and return it so
    // it can be used externally
    template<typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        const char* typeName = typeid(T).name();

        assert(_systems.find(typeName) == _systems.end()
                && "Registering system more than once.");

        auto system = std::make_shared<T>();
        _systems.insert({typeName, system});
        return system;
    }

    // Set the signature for this system
    template<typename T>
    void SetSignature(Signature signature)
    {
        const char* typeName = typeid(T).name();

        assert(_systems.find(typeName) != _systems.end()
                && "System used before registered.");

        _signatures.insert({typeName, signature});
    }

    void EntityDestroyed(Entity entity)
    {
        // Erase a destroyed entity from all system lists
        // _entities is a set so no check needed
        for (const auto& pair : _systems)
        {
            const auto& system = pair.second;
            system->_entities.erase(entity);
        }
    }

    void EntitySignatureChanged(Entity entity, const Signature& entitySignature)
    {
        // Notify each system that an entity's signature changed
        for (const auto& pair : _systems)
        {
            const auto& type = pair.first;
            const auto& system = pair.second;
            const auto& systemSignature = _signatures[type];

            // Entity signature matches system signature
            // Insert into set
            if ((entitySignature & systemSignature) == systemSignature)
            {
                system->_entities.insert(entity);
            }
            // Entity signature does not match system signature
            // Erase from set
            else
            {
                system->_entities.erase(entity);
            }
        }
    }

 private:
     // Map from system type string pointer to a signature
     std::unordered_map<const char*, Signature> _signatures{};

     // Map from system type string pointer to a system pointer
     std::unordered_map<const char*, std::shared_ptr<System>> _systems{};
};
