#pragma once

#include "Component.h"
#include <string>

class Label : public Component
{
public:
	std::string text;


	Label(const std::string &text, const glm::ivec2& position);
	void draw(MenuOverlay* overlay) override;
	bool click(bool, const glm::ivec2 &) override { return false; };
};