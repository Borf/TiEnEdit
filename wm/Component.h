#pragma once

#include <glm/vec2.hpp>
#include <list>
#include <string>

class MenuOverlay;

class DragProperties
{
public:
	enum class Type
	{
		Model,
		Texture,
		Prefab,
		Other,
	} type;
	std::string file;
};

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
	bool focusable = false;
	glm::ivec2 size;
	glm::ivec2 position;
	virtual void onReposition(Component* parent) {
		this->absPosition = parent->absPosition + position;
	};
	virtual void draw(MenuOverlay* overlay) {};
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos, int clickCount) = 0;

	virtual bool mouseDown(bool leftButton, const glm::ivec2 &mousePos) { return false; }
	virtual bool mouseUp(bool leftButton, const glm::ivec2 &mousePos) { return false; }
	virtual bool mouseDrag(bool leftButton, const glm::ivec2 &startPos, const glm::ivec2 &mousePos, const glm::ivec2 &lastMousePos) { return false; }
	virtual bool mouseFinishDrag(bool leftButton, const glm::ivec2 &startPos, const glm::ivec2 &mousePos) { return false; }

	virtual bool scroll(float offset) { return false; };
	virtual bool scrollRecursive(const glm::ivec2 &mousePos, float direction);
	virtual Component* getComponentAt(const glm::ivec2 &pos) { return inComponent(pos) ? this : nullptr; }

	virtual inline bool inComponent(const glm::ivec2 &pos) { return pos.x > absPosition.x && pos.x < absPosition.x + size.x && pos.y > absPosition.y && pos.y < absPosition.y + size.y; }

	virtual void focus() {};
	virtual void unfocus() {};
	virtual bool keyChar(char character) { return false; };
	virtual bool keyUp(int key) { return false;  };
	virtual bool keyDown(int key) { return false; };

	virtual void handleDrag(DragProperties* draggedObject) {};


	static std::list<int*> scissorStack;
	static void scissorPush(int x, int y, int width, int height);
	static void scissorPop();

};