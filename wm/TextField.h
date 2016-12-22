#pragma once

#include "Component.h"
#include <string>
#include <functional>
#include <VrLib/tien/Component.h>

namespace vrlib { class TrueTypeFont;  }

class TextField : public Component, public vrlib::tien::EditorBuilder::TextComponent
{
public:
	std::string value;
	int cursor;
	int selectionEnd;

	float offsetX = 0;
	vrlib::TrueTypeFont* font = nullptr;

	TextField(const std::string &value, glm::ivec2 position);

	void draw(MenuOverlay * overlay);
	virtual bool mouseDown(bool leftButton, const glm::ivec2 & clickPos) override;
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos, int clickCount) override;


	virtual bool keyChar(char character) override;
	virtual bool keyUp(int keyCode) override;
	virtual bool keyDown(int keyCode) override;

	std::function<void()> onChange;

	inline std::string getText() const override { return value; }
	inline void setText(const std::string &text) override { value = text; }
	virtual bool mouseDrag(bool leftButton, const glm::ivec2 &startPos, const glm::ivec2 &mousePos) override;
};