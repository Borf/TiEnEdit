#pragma once

#include "Component.h"

#include <vector>

class MenuOverlay;

class Panel : public Component
{
public:
	std::vector<Component*> components;


	void draw(MenuOverlay* overlay) override;
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos) override;
	virtual void onReposition(Component* parent) override;

};