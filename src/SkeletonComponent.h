#pragma once

#include "ECS.h"
#include "Shader.h"
#include "TransformComponent.h"
#include "DrawableComponent.h"
#include "MarchingCubeComponent.h"
#include "glm/gtx/string_cast.hpp"
#include "src/Renderer.h"
#include <GL/gl.h>
#include <cstdint>
#include <functional>
#include <memory>
#include "shapes.h"
#include "Bone.h"

class SkeletonComponent : public Component
{
public:
	SkeletonComponent(std::shared_ptr<Renderer> __renderer, std::shared_ptr<ECS_Manager> __ECS_manager)
	: Component{}
	, _root { __ECS_manager->addEntity() }
	, _renderer { __renderer }
	, _ECS_manager { __ECS_manager }
	, _skeletonDraw { _ECS_manager->addEntity() }
	{
		_root.addComponent<TransformComponent>(glm::vec3{0,0,-4.5f}, glm::vec3{0,0,0}, glm::vec3{0.1f,0.1f,0.1f});
		_root.addComponent<BoneComponent>();
		auto shader = std::make_shared<Shader>(Shader{"shaders/vert.vert", "shaders/frag.frag"});
		Cube c;
		_root.addComponent<DrawableComponent>(_renderer, shader, c.vertices, c.normals, c.indices, GL_TRIANGLES);
		_bones.emplace_back(&_root);

		_skeletonDraw.addComponent<TransformComponent>(glm::vec3{0,0,0}, glm::vec3{0,0,0}, glm::vec3{1,1,1});
		_skeletonDraw.addComponent<DrawableComponent>(_renderer, shader, vertices(), normals(), indices(), GL_LINES);
	}

	std::vector<GLfloat> vertices()
	{
		auto vertices = _root.getComponent<BoneComponent>().getVertices();
		return vertices;
	}
	std::vector<GLfloat> normals() const
	{
		return {0};
	}
	std::vector<GLuint> indices()
	{
		auto indices = _root.getComponent<BoneComponent>().getIndices(0);
		return indices;
	}
	glm::vec3 position() const
	{
		return _bones[_selectedBone]->getComponent<TransformComponent>().position();
	}
	void changeSelectedBone(std::int64_t diff)
	{
		std::cout << diff << std::endl;
		_selectedBone += diff;	
		_selectedBone %= _bones.size();
	}
	
	void addBone(glm::vec3 pos)
	{
		Cube c;
		auto shader = std::make_shared<Shader>(Shader{"shaders/vert.vert", "shaders/frag.frag"});
		Entity* bone = &_ECS_manager->addEntity();
		bone->addComponent<TransformComponent>(pos, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.1f, 0.1f, 0.1f});
		bone->addComponent<BoneComponent>();
		bone->addComponent<DrawableComponent>(_renderer, shader, c.vertices, c.normals, c.indices, GL_TRIANGLES);
		_root.getComponent<BoneComponent>().addChild(bone);
		_bones.emplace_back(bone);
	}

	void init() override
	{
		addBone({0,0,0});
		addBone({0,0,4.5f});

		auto vertices = entity->getComponent<DrawableComponent>().vertices();
		_root.getComponent<BoneComponent>().setWeights(_weights, vertices);
		
		for (const auto& bw : _weights)
		{
			for (const auto& w : bw)
			{
				std::cout << w << " ";
			}
			std::cout << std::endl << "." << std::endl;
		}

		_renderer->setSkeleton(this);
	}

	void draw() override
	{
	}

	void update([[maybe_unused]] double __deltaTime) override
	{
		_skeletonDraw.getComponent<DrawableComponent>().setVertices(vertices());
		_skeletonDraw.getComponent<DrawableComponent>().setIndices(indices());
		_skeletonDraw.getComponent<DrawableComponent>().updateGeometry();
	}
	

private:
	Cube _c;
	Entity& _root;
	Entity _b1;
	std::vector<std::vector<float>> _weights;
	std::shared_ptr<Renderer> _renderer;
	std::shared_ptr<ECS_Manager> _ECS_manager;
	Entity& _skeletonDraw;
	std::vector<Entity*> _bones;
	std::uint64_t _selectedBone = 0;
};
