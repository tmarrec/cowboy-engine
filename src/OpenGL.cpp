#include "OpenGL.h"
#include "shapes/Triangle.h"

#include <iostream>

OpenGL::OpenGL(int w, int h)
		: _width{w}
		, _height{h}
		, _draw_fill{true}
		, _projection{1.0f}
		, _view_position{0.0f, 0.0f, -5.0f}
{

	_projection = glm::perspective(glm::radians(70.0f), (float)width()/(float)height(), 0.1f, 100.0f);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, _width, _height);

	Triangle t1 {glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec3{0.0f, 0.0f, 0.0f}, glm::vec2{1.0f, 1.0f}};
	Triangle t2 {glm::vec3{3.0f, 0.0f, 0.0f}, glm::vec3{5.0f, 80.0f, 0.0f}, glm::vec2{1.0f, 1.0f}};

	test.push_back(std::make_unique<Entity>(std::move(t1)));
	test.push_back(std::make_unique<Entity>(std::move(t2)));
}

OpenGL::~OpenGL(void) {

}

void OpenGL::draw(void) {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	if (_draw_fill) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	// Render tout ici
	for (auto & t : test) {
		t->draw(view_position(), projection(), delta_time());
	}
	
	//test.draw(view_position(), projection(), delta_time());
	//test2.draw(view_position(), projection(), delta_time());
	
}

unsigned short OpenGL::width() const {
	return _width;
}

unsigned short OpenGL::height() const {
	return _height;
}

glm::mat4 OpenGL::projection() const {
	return _projection;
}

float OpenGL::delta_time() const {
	return _delta_time;
}

void OpenGL::set_delta_time(float delta_time) {
	_delta_time = delta_time;
}

glm::vec3 OpenGL::view_position() const {
	return _view_position;
}
