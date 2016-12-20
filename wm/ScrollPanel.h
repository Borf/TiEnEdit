#pragma once

#include "Component.h"

#include <vector>

class MenuOverlay;

class ScrollPanel : public Component
{
public:
	glm::vec2 scrollOffset;
	Component* component;


	ScrollPanel(Component* component);

	void draw(MenuOverlay* overlay) override;
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos, int clickCount) override;
	virtual void onReposition(Component* parent) override;
	virtual Component* getComponentAt(const glm::ivec2 &pos) override;

	virtual bool scroll(float offset);

};