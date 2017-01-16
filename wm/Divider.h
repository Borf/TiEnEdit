#pragma once

#include "Component.h"
#include <string>
#include "../EditorBuilderGui.h"

class Divider : public Component
{
public:
	Divider(const glm::ivec2& position);
	void draw(MenuOverlay* overlay) override;
	bool click(bool, const glm::ivec2 &, int clickCount) override { return false; };
};