#pragma once

#include "Component.h"
#include <vector>


class SplitPanel : public Component
{
public:
	std::vector<int> sizes;
	std::vector<Component*> panels;

	SplitPanel();

	void addPanel(Component* panel);

	void onReposition();
	void draw(MenuOverlay* overlay) override;
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos) override;
};