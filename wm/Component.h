#pragma once

#include <glm/vec2.hpp>

class MenuOverlay;

class Component
{
	struct Positioning
	{
		glm::ivec2 absPosition;
		enum HorAlignment
		{
			LEFT,
			RIGHT
		};
		HorAlignment left;
		HorAlignment right;
		enum VerAlignment
		{
			TOP,
			BOTTOM
		};
		HorAlignment top;
		HorAlignment bottom;
	} positioning;

public:
	glm::ivec2 size;
	glm::ivec2 position;
	virtual void onReposition(Component* parent) {};
	virtual void draw(MenuOverlay* overlay) {};
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos) = 0;

	inline bool inComponent(const glm::ivec2 &pos) { return pos.x > position.x && pos.x < position.x + size.x && pos.y > position.y && pos.y < position.y + size.y; }

};