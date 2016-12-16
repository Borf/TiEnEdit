#pragma once

#include "Component.h"
#include <string>
#include <vector>
#include <functional>

class ComboBox : public Component
{
public:
	std::string value;
	std::vector<std::string> values;
	std::function<void()> onChange;

	ComboBox(const std::string &value, glm::ivec2 position);

	void draw(MenuOverlay * overlay);
	virtual bool click(bool leftButton, const glm::ivec2 & clickPos, int clickCount) override;
	inline bool inComponent(const glm::ivec2 &pos) override;

	virtual void focus() override { focussed = !focussed; }; // this is because focussed is also toggled in click

};