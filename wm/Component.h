#pragma once

#include <glm/vec2.hpp>

class MenuOverlay;

class Component
{
public:
	glm::ivec2 absPosition;

	struct Alignment
	{
		enum HorAlignment
		{
			LEFT,
			RIGHT
		};
		HorAlignment left = LEFT;
		HorAlignment right = LEFT;
		enum VerAlignment
		{
			TOP,
			BOTTOM
		};
		VerAlignment top = TOP;
		VerAlignment bottom = TOP;
	} alignment;

	bool focussed = false;
	glm::ivec2 size;
	glm::ivec2 position;
	virtual void onReposition(Component* parent);
	virtual void draw(MenuOverlay* overlay) {};
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos) = 0;
	virtual Component* getComponentAt(const glm::ivec2 &pos) { return inComponent(pos) ? this : nullptr; }

	virtual inline bool inComponent(const glm::ivec2 &pos) { return pos.x > absPosition.x && pos.x < absPosition.x + size.x && pos.y > absPosition.y && pos.y < absPosition.y + size.y; }

	virtual void focus() {};
	virtual void unfocus() {};
	virtual void keyChar(char character) {};
	virtual void keyUp(int key) {};

};