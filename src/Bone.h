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
	BoneComponent()
	: Component{}
	{
	}
	std::vector<Entity*>& childs() { return _childs; }

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

	void setWeights(std::vector<std::vector<float>>& weights, std::shared_ptr<std::vector<GLfloat>> vertices)
	{
		// Own weight
		std::vector<float> boneWeights;
		double sum = 0.0f;
		for (std::uint64_t i = 0; i < vertices->size(); i += 3)
		{
			glm::vec3 vertPos = {(*vertices)[i], (*vertices)[i+1], (*vertices)[i+2]};
			glm::vec3 bonePos = entity->getComponent<TransformComponent>().position();
			float w = 1.0f/std::pow(glm::distance(vertPos, bonePos), 2);
			boneWeights.emplace_back(w);
			sum += w;
		}
		// Normalize
		for (auto& w : boneWeights)
			w = w / sum;
		weights.emplace_back(boneWeights);

		// Childs weights
		for (auto it = _childs.begin(); it != _childs.end(); ++it)
		{
			(*it)->getComponent<BoneComponent>().setWeights(weights, vertices);
		}
	}

	void addChild(Entity* child) { _childs.emplace_back(child); }

	void move(glm::mat4 transformMatrix, glm::vec3 origin)
	{
		entity->getComponent<TransformComponent>().applyTransformMatrix(transformMatrix);

		for (auto it = _childs.begin(); it != _childs.end(); ++it)
		{

			//(*it)->getComponent<BoneComponent>().move(transformMatrix);

			auto p1 = origin;
			auto p2 = (*it)->getComponent<TransformComponent>().position();
			glm::vec3 res = transformMatrix * glm::vec4(p2-p1, 1);
			(*it)->getComponent<TransformComponent>().move(res-(p2-p1));


			/*
			std::cout << std::endl;
			std::cout << glm::to_string(p1) << std::endl;
			std::cout << glm::to_string(p2) << std::endl;
			std::cout << glm::to_string(p2-p1) << std::endl;
			std::cout << glm::to_string(res) << std::endl;
			std::cout << std::endl;
			*/

			// Propagate
			(*it)->getComponent<BoneComponent>().move(transformMatrix, origin);
		}
		
	}


	void update([[maybe_unused]] double __deltaTime) override
	{
		if (entity->getEntityID() >= 5)
		{
			glm::mat4 model {1.0f};
			model = glm::translate(model, {0, 0, 0});
			model = glm::rotate(model, glm::radians(float(5.0f*__deltaTime)), glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			move(model, entity->getComponent<TransformComponent>().position());
		}
	}

private:
	std::vector<Entity*> _childs;
};

