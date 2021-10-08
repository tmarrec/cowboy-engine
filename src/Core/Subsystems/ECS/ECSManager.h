#pragma once

#include "./ComponentManager.h"
#include "./EntityManager.h"
#include "./SystemManager.h"

#include <memory>

class ECSManager
{
 public:
    // Create new entity and returns it
    Entity CreateEntity()
    {
        return _entityManager->CreateEntity();
    }

    // Destroy entity and warns all the managers
    void DestroyEntity(Entity entity)
    {
        _entityManager->DestroyEntity(entity);
        _componentManager->EntityDestroyed(entity);
        _systemManager->EntityDestroyed(entity);
    }

    template<typename T>
    void RegisterComponent()
    {
        _componentManager->RegisterComponent<T>();
    }

    // Add new component to an entity and warns all the managers
    template<typename T>
    void AddComponent(Entity entity, T component)
    {
        _componentManager->AddComponent<T>(entity, component);
        auto signature = _entityManager->Sig(entity);
        signature.set(_componentManager->GetComponentType<T>(), true);
        _entityManager->Sig(entity) = signature;
        _systemManager->EntitySignatureChanged(entity, signature);
    }

    // Removes component to an entity and warns all the managers
    template<typename T>
    void RemoveComponent(Entity entity)
    {
        _componentManager->RemoveComponent<T>(entity);
        auto signature = _entityManager->Sig(entity);
        signature.set(_componentManager->GetComponentType<T>(), false);
        _entityManager->Sig(entity) = signature;
        _systemManager->EntitySignatureChanged(entity, signature);
    }

    template<typename T>
    T& GetComponent(Entity entity)
    {
        return _componentManager->GetComponent<T>(entity);
    }

    template<typename T>
    ComponentType GetComponentType()
    {
        return _componentManager->GetComponentType<T>();
    }

    template<typename T>
    std::shared_ptr<T> RegisterSystem()
    {
        return _systemManager->RegisterSystem<T>();
    }

    template<typename T>
    void SetSystemSignature(Signature signature)
    {
        _systemManager->SetSignature<T>(signature);
    }

 private:
    std::unique_ptr<ComponentManager> _componentManager
        = std::make_unique<ComponentManager>();
    std::unique_ptr<EntityManager> _entityManager
        = std::make_unique<EntityManager>();
    std::unique_ptr<SystemManager> _systemManager
        = std::make_unique<SystemManager>();
};
