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
	BoneComponent(glm::vec3 parentPos, float __size)
	: Component{}
	, _parentPos { parentPos }
	, _root { false }
	, _undeformedTransform { 0.0f }
	, _deformedTransform { 0.0f }
	, _size { __size }
	{
	}
	BoneComponent(float __size)
	: Component{}
	, _parentPos { glm::vec3{0, 0, 0} }
	, _root { true }
	, _undeformedTransform { 0.0f }
	, _deformedTransform { 0.0f }
	, _size { __size }
	{
	}
	std::vector<Entity*>& childs() { return _childs; }
	glm::mat4 undeformedTransform() { return _undeformedTransform; }
	glm::mat4 deformedTransform() { return _deformedTransform; }
	bool root() { return _root; }
	glm::vec3 parentPos() { return _parentPos; }
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
			for (std::uint64_t i = 0; i < vertices->size(); i += 3)
			{
				glm::vec3 vertPos = {(*vertices)[i], (*vertices)[i+1], (*vertices)[i+2]};
				glm::vec3 bonePos = position();
		
				float dist = glm::distance(vertPos, bonePos);
				if (dist == 0.0f)
					dist = 0.00001f;
				float w = 1.0f/std::pow(dist, 2);
				boneWeights.emplace_back(w);
			}

			weights.emplace_back(boneWeights);
		}

		// Childs weights
		for (auto it = _childs.begin(); it != _childs.end(); ++it)
		{
			(*it)->getComponent<BoneComponent>().setWeights(weights, vertices);
		}
	}

	void move(glm::mat4 transform)
	{
		setDeformedTransform(_deformedTransform*transform);
		// Bone transform
		//entity->getComponent<TransformComponent>().applyTransformMatrix(transform);
		std::cout << glm::to_string(_parentPos) << std::endl;
		glm::vec3 newPos = _deformedTransform * glm::vec4(_parentPos, 1);
		entity->getComponent<TransformComponent>().setPosition(newPos);

		// Propagate
		for (auto it = _childs.begin(); it != _childs.end(); ++it)
		{
			(*it)->getComponent<BoneComponent>().move(transform);
		}
	}

	void reset()
	{
		setDeformedTransform(_undeformedTransform);
		glm::mat4 transform {1.0f};
		transform = _undeformedTransform;
		entity->getComponent<TransformComponent>().setPosition(glm::vec3{0,0,0});
		entity->getComponent<TransformComponent>().applyTransformMatrix(transform);
	}

	void addChild(Entity* child) { _childs.emplace_back(child); }
	float size() { return _size; }


private:
	std::vector<Entity*> _childs;
	glm::vec3 _parentPos;
	bool _root;
	glm::mat4 _undeformedTransform;
	glm::mat4 _deformedTransform;
	float _size;
};

