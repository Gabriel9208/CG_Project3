#pragma once
#include "../include/glm/glm.hpp"
#include "../include/glm/gtx/transform.hpp"
#include "../Graphic/UBO.h"

#include <GL/glew.h>

class Light
{
private:
	glm::vec3 position;
	glm::vec3 color;
	float shininess;

	UBO ubo;

public:
	Light() : position(0, 10, 10), shininess(32.0), color(1.0, 1.0, 1.0) {}
	~Light() {}

	void initialize();
	void bind(GLuint programId);
	void setPosition(glm::vec3 pos);
	void setShininess(float shine);
	void setColor(glm::vec3 _color);

	inline glm::vec3 getPos() const { return position; }
	inline float getShininess() const { return shininess; }
	inline glm::vec3 getColor() const { return color; }
};