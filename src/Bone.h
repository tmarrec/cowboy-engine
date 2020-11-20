#pragma once 

#include "ECS.h"
#include "glm/gtx/dual_quaternion.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/trigonometric.hpp"
#include "src/TransformComponent.h"
#include <GL/gl.h>
#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>


class BoneComponent : public Component
{
public:
	BoneComponent()
	: Component{}
	, _weight { 0 }
	{
	}

	BoneComponent(float weight)
	: Component{}
	, _weight { weight }
	{
	}
	float weight() const { return _weight; }
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

	void addChild(Entity* child) { _childs.emplace_back(child); }

	void move(glm::vec3 moveVec, glm::vec3 rotaVec)
	{
		//entity->getComponent<TransformComponent>().move(moveVec);
		entity->getComponent<TransformComponent>().rotate(rotaVec);

		for (auto it = _childs.begin(); it != _childs.end(); ++it)
		{
			auto p1 = entity->getComponent<TransformComponent>().position();
			auto p2 = (*it)->getComponent<TransformComponent>().position();

			auto x = glm::rotateX(p1-p2, glm::radians(rotaVec.x));
			auto y = glm::rotateY(x, glm::radians(rotaVec.y));
			auto res = glm::rotateZ(y, glm::radians(rotaVec.z));

			std::cout << glm::to_string(p1) << std::endl;
			std::cout << glm::to_string(p2) << std::endl;
			std::cout << glm::to_string(res) << std::endl;
			std::cout << std::endl;
			(*it)->getComponent<TransformComponent>().setPosition(-res);

			(*it)->getComponent<BoneComponent>().move({0,0,0}, {0,0,0});
		}
	}


	void update(double __deltaTime) override
	{
		if (entity->getEntityID() == 5)
		{
			move({0.0f, 0.05f*__deltaTime, 0.0f},{0.5f*__deltaTime, 0.0f, 0.0f});
		}
	}

private:
	float _weight;
	std::vector<Entity*> _childs;
};

