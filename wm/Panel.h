#pragma once

#include "Component.h"

class MenuOverlay;

class Panel : public Component
{
public:
	void draw(MenuOverlay* overlay) override;
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos) override;
};