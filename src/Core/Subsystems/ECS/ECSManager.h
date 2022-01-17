#pragma once

#include "./ComponentManager.h"
#include "./EntityManager.h"
#include "./SystemManager.h"

class ECSManager
{
 public:
    // Create new entity and returns it
    Entity createEntity()
    {
        return _entityManager->createEntity();
    }

    // Destroy entity and warns all the managers
    void destroyEntity(Entity entity)
    {
        _entityManager->destroyEntity(entity);
        _componentManager->entityDestroyed(entity);
        _systemManager->entityDestroyed(entity);
    }

    template<typename T>
    void registerComponent()
    {
        _componentManager->registerComponent<T>();
    }

    // Add new component to an entity and warns all the managers
    template<typename T>
    void addComponent(Entity entity, T component)
    {
        _componentManager->addComponent<T>(entity, component);
        auto signature = _entityManager->sig(entity);
        signature.set(_componentManager->getComponentType<T>(), true);
        _entityManager->sig(entity) = signature;
        _systemManager->entitySignatureChanged(entity, signature);
    }

    // Removes component to an entity and warns all the managers
    template<typename T>
    void removeComponent(Entity entity)
    {
        _componentManager->removeComponent<T>(entity);
        auto signature = _entityManager->sig(entity);
        signature.set(_componentManager->getComponentType<T>(), false);
        _entityManager->sig(entity) = signature;
        _systemManager->entitySignatureChanged(entity, signature);
    }

    template<typename T>
    T& getComponent(Entity entity)
    {
        return _componentManager->getComponent<T>(entity);
    }

    template<typename T>
    ComponentType getComponentType()
    {
        return _componentManager->getComponentType<T>();
    }

    template<typename T>
    std::shared_ptr<T> registerSystem()
    {
        return _systemManager->registerSystem<T>();
    }

    template<typename T>
    void setSystemSignature(Signature signature)
    {
        _systemManager->setSignature<T>(signature);
    }

 private:
    std::unique_ptr<ComponentManager> _componentManager = std::make_unique<ComponentManager>();
    std::unique_ptr<EntityManager>    _entityManager    = std::make_unique<EntityManager>();
    std::unique_ptr<SystemManager>    _systemManager    = std::make_unique<SystemManager>();
};
