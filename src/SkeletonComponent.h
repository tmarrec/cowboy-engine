#pragma once

#include "ECS.h"
#include "TransformComponent.h"
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
	SkeletonComponent()
	: Component{}
	, _root { Bone{1, {0, 0, 0}} }
	{
	}

	std::vector<GLfloat> vertices()
	{
		auto vertices = _root.getVertices();
		for(const auto& v : vertices)
			std::cout << v << " ";
		std::cout << std::endl;
		return vertices;
	}
	std::vector<GLfloat> normals() const
	{
		return {0};
	}
	std::vector<GLuint> indices()
	{
		auto indices = _root.getIndices(0);
		for(const auto& i : indices)
			std::cout << i << " ";
		std::cout << std::endl;
		return indices;
	}

	void init() override
	{
		std::shared_ptr<Bone> b1 = std::make_shared<Bone>(Bone {10, {-1,0,-1}});
		std::shared_ptr<Bone> b2 = std::make_shared<Bone>(Bone {100, {1,0,-1}});
		std::shared_ptr<Bone> b3 = std::make_shared<Bone>(Bone {1000, {-2,0,-2}});
		std::shared_ptr<Bone> b4 = std::make_shared<Bone>(Bone {10000, {-1,0,-2}});
		std::shared_ptr<Bone> b5 = std::make_shared<Bone>(Bone {100000, {0,0,-2}});
		std::shared_ptr<Bone> b6 = std::make_shared<Bone>(Bone {1000000, {-1,0,-3}});
		_root.addChild(b1);
		_root.addChild(b2);
		b1->addChild(b3);
		b1->addChild(b4);
		b1->addChild(b5);
		b4->addChild(b6);
	}

	void draw() override
	{
	}

	void update([[maybe_unused]] double __deltaTime) override
	{
	}
	

private:
	Cube _c;
	Bone _root;
};
