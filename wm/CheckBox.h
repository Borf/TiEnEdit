#pragma once

#include "Component.h"
#include <string>
#include <functional>

class CheckBox : public Component
{
public:
	bool value;

	std::function<void()> onChange;


	CheckBox(bool value, glm::ivec2 position);

	void draw(MenuOverlay * overlay);
	virtual bool click(bool leftButton, const glm::ivec2 & clickPos, int clickCount) override;


};