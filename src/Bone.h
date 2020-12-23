#pragma once 

#include "ECS.h"
#include "glm/gtx/dual_quaternion.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/trigonometric.hpp"
#include "src/TransformComponent.h"
#include <GL/gl.h>
#include <cstdint>
#include <iterator>
#include <memory>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>


class BoneComponent : public Component
{
public:
	BoneComponent(glm::vec3 parentPos)
	: Component{}
	, _parentPos { parentPos }
	, _root { false }
	, _undeformedTransform { 1.0f }
	, _deformedTransform { 1.0f }
	{
	}
	BoneComponent()
	: Component{}
	, _parentPos { glm::vec3{0, 0, 0} }
	, _root { true }
	, _undeformedTransform { 1.0f }
	, _deformedTransform { 1.0f }
	{
	}
	std::vector<Entity*>& childs() { return _childs; }
	glm::mat4 undeformedTransform() { return _undeformedTransform; }
	glm::mat4 deformedTransform() { return _deformedTransform; }
	bool root() { return _root; }
	glm::vec3 position()
	{
		glm::vec3 pos = entity->getComponent<TransformComponent>().position();
		return (pos+_parentPos)/glm::vec3{2,2,2};
	}

	std::vector<GLuint> getIndices(std::uint64_t ind)
	{
		std::vector<GLuint> res;
		std::uint64_t shift = 1;
		for (auto it = _childs.begin(); it != _childs.end(); ++it)
		{
			res.emplace_back(ind);
			res.emplace_back(ind+shift);
			auto indices = (*it)->getComponent<BoneComponent>().getIndices(ind+shift);
			shift = indices.size() > 0 ? indices.back()+1-ind : shift+1;
			res.insert(res.end(), indices.begin(), indices.end());
		}
		return res;
	}

	std::vector<GLfloat> getVertices()
	{
		std::vector<GLfloat> res;
		auto pos = entity->getComponent<TransformComponent>().position();
		res.emplace_back(pos.x);
		res.emplace_back(pos.y);
		res.emplace_back(pos.z);
		for (auto it = _childs.begin(); it != _childs.end(); ++it)
		{
			auto vertices = (*it)->getComponent<BoneComponent>().getVertices();
			res.insert(res.end(), vertices.begin(), vertices.end());
		}
		return res;
	}

	void setUndeformedTransform(glm::mat4 transform)
	{
		_undeformedTransform = transform;
	}

	void setDeformedTransform(glm::mat4 transform)
	{
		_deformedTransform = transform;
	}

	void setWeights(std::vector<std::vector<float>>& weights, std::shared_ptr<std::vector<GLfloat>> vertices)
	{
		if (!_root)
		{
			// Own weight
			std::vector<float> boneWeights;
			double sum = 0.0f;
			for (std::uint64_t i = 0; i < vertices->size(); i += 3)
			{
				glm::vec3 vertPos = {(*vertices)[i], (*vertices)[i+1], (*vertices)[i+2]};
				glm::vec3 bonePos = position();
		
				float dist = glm::distance(vertPos, bonePos);
				if (dist == 0.0f)
					dist = 0.00001f;
				float w = 1.0f/std::pow(dist, 2);
				boneWeights.emplace_back(w);
				sum += w;
			}
			// Normalize
			for (auto& w : boneWeights)
				w = w / sum;
	
			weights.emplace_back(boneWeights);
		}

		// Childs weights
		for (auto it = _childs.begin(); it != _childs.end(); ++it)
		{
			(*it)->getComponent<BoneComponent>().setWeights(weights, vertices);
		}
	}

	void addChild(Entity* child) { _childs.emplace_back(child); }

	void move(glm::mat4 transform, glm::vec3 origin)
	{
		setDeformedTransform(_deformedTransform*transform);
		entity->getComponent<TransformComponent>().applyTransformMatrix(transform);

		for (auto it = _childs.begin(); it != _childs.end(); ++it)
		{
			auto p1 = origin;
			auto p2 = (*it)->getComponent<TransformComponent>().position();
			glm::vec3 res = transform * glm::vec4(p2-p1, 1);
			(*it)->getComponent<TransformComponent>().move(res-(p2-p1));

			// Propagate
			(*it)->getComponent<BoneComponent>().move(transform, origin);
		}
		
	}

	void update([[maybe_unused]] double __deltaTime) override
	{
		if (entity->getEntityID() >= 5)
		{
			glm::mat4 model {1.0f};
			model = glm::translate(model, {0, 0, 0});
			model = glm::rotate(model, glm::radians((float)(1.5f*__deltaTime)), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			move(model, entity->getComponent<TransformComponent>().position());
		}
	}

private:
	std::vector<Entity*> _childs;
	glm::vec3 _parentPos;
	bool _root = false;
	glm::mat4 _undeformedTransform;
	glm::mat4 _deformedTransform;
};

