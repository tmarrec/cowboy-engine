#pragma once

#include "ECS.h"
#include "Shader.h"
#include "TransformComponent.h"
#include "DrawableComponent.h"
#include "MarchingCubeComponent.h"
#include "glm/geometric.hpp"
#include "glm/gtx/string_cast.hpp"
#include "src/Renderer.h"
#include <GL/gl.h>
#include <cstdint>
#include <functional>
#include <memory>
#include "shapes.h"
#include "BoneComponent.h"

#include <unistd.h>
#include <iomanip>

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
		// Add root bone
		glm::vec3 pos = {0.0f, -0.2f, 0.0f};
		_root.addComponent<TransformComponent>(pos, glm::vec3{0,0,0}, glm::vec3{0.1f,0.1f,0.1f});
		_root.addComponent<BoneComponent>(1.0f);
		auto shader = std::make_shared<Shader>(Shader{"shaders/vert.vert", "shaders/skeleton.frag"});
		_skeletonDraw.addComponent<TransformComponent>(glm::vec3{0,0,0}, glm::vec3{0,0,0}, glm::vec3{1,1,1});
		_skeletonDraw.addComponent<DrawableComponent>(_renderer, shader, std::vector<GLfloat>{}, std::vector<GLfloat>{}, std::vector<GLuint>{}, GL_TRIANGLES);
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
	glm::vec3 position() const
	{
		return _bones[_selectedBone]->getComponent<BoneComponent>().position();
	}
	glm::vec3 startPosition() const
	{
		return _bones[_selectedBone]->getComponent<BoneComponent>().startPosition();
	}
	glm::vec3 endPosition() const
	{
		return _bones[_selectedBone]->getComponent<BoneComponent>().endPosition();
	}
	float selectedBoneSize()
	{
		return _bones[_selectedBone]->getComponent<BoneComponent>().size();
	}
	void changeSelectedBone(std::int64_t diff)
	{
		_selectedBone += diff;	
		if (_selectedBone == (unsigned long)-1) _selectedBone = _bones.size()-1;
		else _selectedBone %= _bones.size();
	}
	
	Entity* addBone(Entity* parent, glm::mat4 transformMatrix, float size)
	{
		Entity* bone = &_ECS_manager->addEntity();

		glm::vec3 parentPos = parent->getComponent<TransformComponent>().position();
		bone->addComponent<BoneComponent>(parentPos, size);

		glm::vec3 pos = transformMatrix * glm::vec4(parentPos, 1);
		pos += glm::vec3{0, 0, bone->getComponent<BoneComponent>().size()};

		bone->addComponent<TransformComponent>(pos, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.1f, 0.1f, 0.1f});

		parent->getComponent<BoneComponent>().addChild(bone);

		bone->getComponent<BoneComponent>().setUndeformedTransform(transformMatrix);
		bone->getComponent<BoneComponent>().setDeformedTransform(transformMatrix);

		// Bone representation transform
		bone->getComponent<TransformComponent>().setPosition(bone->getComponent<BoneComponent>().endPosition());

		_bones.emplace_back(bone);
		return bone;
	}

	void init() override
	{
		// Default pose skeleton
		glm::mat4 model {1.0f};
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		[[maybe_unused]] auto body = addBone(&_root, model, 0.7f);
		[[maybe_unused]] auto head = addBone(body, model, 1.0f);

		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		[[maybe_unused]] auto left_arm = addBone(body, model, 1.5f);
		[[maybe_unused]] auto left_subarm = addBone(left_arm, model, 1.1f);
		[[maybe_unused]] auto left_hand = addBone(left_subarm, model, 0.55f);

		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		[[maybe_unused]] auto right_arm = addBone(body, model, 1.5f);
		[[maybe_unused]] auto right_subarm = addBone(right_arm, model, 1.1f);
		[[maybe_unused]] auto right_hand = addBone(right_subarm, model, 0.55f);

		auto saveModel = model;

		model = glm::rotate(model, glm::radians(-30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(7.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		[[maybe_unused]] auto right_hip = addBone(&_root, model, 0.60f);
		model = glm::rotate(model, glm::radians(-7.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-60.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		[[maybe_unused]] auto right_leg = addBone(right_hip, model, 0.8f);
		[[maybe_unused]] auto right_subleg = addBone(right_leg, model, 0.7f);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		[[maybe_unused]] auto right_foot = addBone(right_subleg, model, 0.8f);

		model = saveModel;
		model = glm::rotate(model, glm::radians(-150.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(7.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		[[maybe_unused]] auto left_hip = addBone(&_root, model, 0.60f);
		model = glm::rotate(model, glm::radians(-7.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(60.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		[[maybe_unused]] auto left_leg = addBone(left_hip, model, 0.8f);
		[[maybe_unused]] auto left_subleg = addBone(left_leg, model, 0.7f);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		[[maybe_unused]] auto left_foot = addBone(left_subleg, model, 0.8f);
		

		auto vertices = entity->getComponent<DrawableComponent>().modelVertices();
		_root.getComponent<BoneComponent>().setWeights(_weights, vertices);

		// Normalize sum to 1
		// Bone loop
		for (std::uint64_t j = 0; j < _weights[0].size(); ++j)
		{
			double sum = 0.0f;
			for (std::uint64_t i = 0; i < _weights.size(); ++i)
			{
				sum += _weights[i][j];
			}
			for (std::uint64_t i = 0; i < _weights.size(); ++i)
			{
				_weights[i][j] /= sum;
			}
		}
		_renderer->setSkeleton(this);
	}

	// Apply transform to bone
	void moveBone(Entity* bone, glm::mat4 transform)
	{
		bone->getComponent<BoneComponent>().move(transform);
		_moved = true;
	}

	// Animate the skeleton with inputs
	void animation()
	{
		// Animation
		if (_animX || _animY || _animZ)
		{
			glm::mat4 model {1.0f};
			if (_animX)
			{
				model = glm::rotate(model, glm::radians(20.0f), glm::vec3(1.0f, 0.0f, 0.0f));
				switchAnim(0);	
			}
			else if (_animY)
			{
				model = glm::rotate(model, glm::radians(20.0f), glm::vec3(0.0f, 1.0f, 0.0f));
				switchAnim(1);	
			}
			if (_animZ)
			{
				model = glm::rotate(model, glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				switchAnim(2);	
			}
			moveBone(_bones[_selectedBone], model);
		}

	}

	void update([[maybe_unused]] double __deltaTime) override
	{
		animation();

		if (!_moved)
			return;
		_moved = false;

		auto uvertices = entity->getComponent<DrawableComponent>().undeformedVertices();
		auto modele = entity->getComponent<DrawableComponent>().getModel();
		
		// Apply transformation to each vertex
		// Vertex loop
		std::vector<GLfloat> newV;
		for (std::uint64_t j = 0; j < uvertices->size(); j += 3)
		{
			glm::vec3 vpos = {(*uvertices)[j], (*uvertices)[j+1], (*uvertices)[j+2]};

			// Bone loop
			glm::mat4 vTransform {0.0f};
			for (std::uint64_t i = 0; i < _weights.size(); ++i)
			{
				auto w = _weights[i][int(j/3)];
				auto bone = _bones[i]->getComponent<BoneComponent>();
				vTransform += w * (bone.deformedTransform() * glm::inverse(bone.undeformedTransform()));
			}
			glm::vec3 newVpos = vTransform * modele * glm::vec4(vpos, 1);
			
			newV.insert(newV.end(), {newVpos.x, newVpos.y, newVpos.z});
		}
		entity->getComponent<DrawableComponent>().setVerticesAnim(newV);
		entity->getComponent<DrawableComponent>().updateGeometry();

		// Draw skeleton
		std::vector<GLfloat> newVertices;
		for (const auto& b : _bones)
		{
			auto v = b->getComponent<BoneComponent>().getVertices();
			newVertices.insert(newVertices.end(), v.begin(), v.end());
		}
		std::vector<GLuint> newIndices;
		newIndices.resize(newVertices.size()/3);
		std::iota(newIndices.begin(), newIndices.end(), 0);
		_skeletonDraw.getComponent<DrawableComponent>().setVertices(newVertices);
		_skeletonDraw.getComponent<DrawableComponent>().setIndices(newIndices);
		_skeletonDraw.getComponent<DrawableComponent>().updateGeometry();
		// End draw skeleton
	}

	void switchAnim(std::uint8_t dir)
	{
		if (dir == 0)
		{
			if (_animX)
				_animX = false;
			else
				_animX = true;
			_animY = false;
			_animZ = false;
		}
		else if (dir == 1)
		{
			if (_animY)
				_animY = false;
			else
				_animY = true;
			_animX = false;
			_animZ = false;
		}
		else if (dir == 2)
		{
			if (_animZ)
				_animZ = false;
			else
				_animZ = true;
			_animY = false;
			_animX = false;
		}
	}	

	void resetAnim()
	{
		for (const auto& b : _bones)
			b->getComponent<BoneComponent>().reset();		
		glm::mat4 model {1.0f};
		for (auto& c : _root.getComponent<BoneComponent>().childs())
			c->getComponent<BoneComponent>().move(model);
		_moved = true;
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
	bool _animX = false;
	bool _animY = false;
	bool _animZ = false;
	bool _moved = true;
};
