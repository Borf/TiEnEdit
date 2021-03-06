#pragma once

#include "Panel.h"
#include <vector>


class SplitPanel : public Panel
{
public:
	std::vector<int> sizes;
	enum class Alignment
	{
		HORIZONTAL,
		VERTICAL,
	} alignment = Alignment::HORIZONTAL;

	SplitPanel(Alignment alignment);

	void addPanel(Component* panel);

	void onReposition(Component* parent) override;
	void draw(MenuOverlay* overlay) override;
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos, int clickCount) override;
	virtual bool mouseUp(bool leftButton, const glm::ivec2 &clickPos) override;
};