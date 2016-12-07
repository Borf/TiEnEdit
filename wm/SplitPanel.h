#pragma once

#include "Panel.h"
#include <vector>


class SplitPanel : public Panel
{
public:
	std::vector<int> sizes;

	SplitPanel();

	void addPanel(Component* panel);

	void onReposition();
	void draw(MenuOverlay* overlay) override;
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos) override;
};