#pragma once 

#include <GL/gl.h>
#include <cstdint>
#include <memory>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>


class Bone
{
public:
	Bone(float weight, glm::vec3 pos)
	: _weight { weight }
	, _position { pos }
	{
		_position += glm::vec3{0, 20, 0};
	}
	float weight() const { return _weight; }
	std::vector<std::shared_ptr<Bone>>& childs() { return _childs; }


	std::vector<GLuint> getIndices(std::uint64_t ind)
	{
		std::vector<GLuint> res;
		std::uint64_t childInd = 1;
		std::uint64_t shift = 1;
		for (auto it = _childs.begin(); it != _childs.end(); ++it)
		{
			res.emplace_back(ind);
			res.emplace_back(ind+shift);
			auto indices = (*it)->getIndices(ind+shift);
			std::cout << indices.size() << std::endl;
			shift = indices.size() > 0 ? indices.back()+1-ind : shift+1;
			res.insert(res.end(), indices.begin(), indices.end());

		}
		return res;
	}

	std::vector<GLfloat> getVertices()
	{
		std::vector<GLfloat> res;
		res.emplace_back(_position.x);
		res.emplace_back(_position.y);
		res.emplace_back(_position.z);
		for (auto it = _childs.begin(); it != _childs.end(); ++it)
		{
			auto vertices = (*it)->getVertices();
			res.insert(res.end(), vertices.begin(), vertices.end());
		}
		return res;
	}

	void addChild(std::shared_ptr<Bone> child) { _childs.emplace_back(child); }

private:
	float _weight;
	glm::vec3 _position;
	std::vector<std::shared_ptr<Bone>> _childs;
};

