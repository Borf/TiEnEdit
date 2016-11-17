#pragma once

#include <glm/vec2.hpp>

class Component
{
public:
	//virtual void draw() = 0;


	glm::ivec2 size;
	glm::ivec2 position;
	virtual void onReposition() {};

};