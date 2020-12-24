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
		return (pos+_parentPos)*0.5f;
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
		glm::vec3 start = {0, 0, 0};
		glm::vec3 end = {0, 0, _size};
		start = glm::vec3(_deformedTransform * glm::vec4(start, 1)) + _parentPos;
		end = glm::vec3(_deformedTransform * glm::vec4(end, 1)) + _parentPos;

		float dist = glm::distance(start, end);
		float r = dist/25;
		glm::vec3 s1 = start + glm::vec3(_deformedTransform * glm::vec4{r, r, r, 1});
		glm::vec3 s2 = start + glm::vec3(_deformedTransform * glm::vec4{r, -r, r, 1});
		glm::vec3 s3 = start + glm::vec3(_deformedTransform * glm::vec4{-r, -r, r, 1});
		glm::vec3 s4 = start + glm::vec3(_deformedTransform * glm::vec4{-r, r, r, 1});

		std::uint64_t oldSize = res.size();
		res.resize(oldSize+9);
		memcpy(&res[oldSize], &start, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+3], &s1, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+6], &s2, sizeof(GLfloat)*3);
		oldSize = res.size();
		res.resize(oldSize+9);
		memcpy(&res[oldSize], &start, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+3], &s2, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+6], &s3, sizeof(GLfloat)*3);
		oldSize = res.size();
		res.resize(oldSize+9);
		memcpy(&res[oldSize], &start, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+3], &s3, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+6], &s4, sizeof(GLfloat)*3);
		oldSize = res.size();
		res.resize(oldSize+9);
		memcpy(&res[oldSize], &start, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+3], &s4, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+6], &s1, sizeof(GLfloat)*3);

		oldSize = res.size();
		res.resize(oldSize+9);
		memcpy(&res[oldSize], &end, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+3], &s1, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+6], &s2, sizeof(GLfloat)*3);
		oldSize = res.size();
		res.resize(oldSize+9);
		memcpy(&res[oldSize], &end, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+3], &s2, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+6], &s3, sizeof(GLfloat)*3);
		oldSize = res.size();
		res.resize(oldSize+9);
		memcpy(&res[oldSize], &end, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+3], &s3, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+6], &s4, sizeof(GLfloat)*3);
		oldSize = res.size();
		res.resize(oldSize+9);
		memcpy(&res[oldSize], &end, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+3], &s4, sizeof(GLfloat)*3);
		memcpy(&res[oldSize+6], &s1, sizeof(GLfloat)*3);

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

	void setParentPos(glm::vec3 parentPos)
	{
		_parentPos = parentPos;
	}

	void move(glm::mat4 transform)
	{
		setDeformedTransform(_deformedTransform*transform);
		// Bone representation transform
		glm::vec3 end = glm::vec3(_deformedTransform * glm::vec4{0, 0, _size, 1}) + _parentPos;
		entity->getComponent<TransformComponent>().setPosition(end);

		// Propagate
		for (auto it = _childs.begin(); it != _childs.end(); ++it)
		{
			// Used for bone representation
			(*it)->getComponent<BoneComponent>().setParentPos(end);
			// Used for vertices transformations
			(*it)->getComponent<BoneComponent>().move(transform);
		}
	}

	void reset()
	{
		setDeformedTransform(_undeformedTransform);
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

