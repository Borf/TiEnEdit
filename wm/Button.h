#pragma once

#include "Component.h"
#include <string>
#include <functional>

class Button : public Component
{
public:
	std::string text;

	std::function<void()> onClick;


	Button(const std::string &text, glm::ivec2 position);

	void draw(MenuOverlay * overlay);
	virtual bool click(bool leftButton, const glm::ivec2 & clickPos) override;


};