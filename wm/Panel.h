#pragma once

#include "Component.h"

#include <vector>

class MenuOverlay;

class Panel : public Component
{
public:
	std::vector<Component*> components;


	void draw(MenuOverlay* overlay) override;
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos, int clickCount) override;
	virtual void onReposition(Component* parent) override;
	virtual Component* getComponentAt(const glm::ivec2 &pos) override;
	virtual bool scrollRecursive(const glm::ivec2 &mousePos, float direction) override;
};