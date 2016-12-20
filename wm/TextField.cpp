#include "TextField.h"

#include "../menu/MenuOverlay.h"
#include <VrLib/Kernel.h>
#include <VrLib/Font.h>

#include <Windows.h> // TODO: keycodes

TextField::TextField(const std::string & value, glm::ivec2 position)
{
	this->value = value;
	this->position = position;
}


void TextField::draw(MenuOverlay * overlay)
{

	overlay->drawRect(glm::vec2(128, 328), glm::vec2(128 + 37, 328 + 33), absPosition, absPosition + size); //text background
	overlay->flushVerts();

	if (focussed)
	{
		overlay->shader->setUniform(MenuOverlay::Uniforms::colorMult, glm::vec4(0.1f, 0.1f, 1, 1)); //TODO
		overlay->drawRect(glm::vec2(8, 328), glm::vec2(8 + 37, 328 + 33), absPosition, absPosition + size);
		overlay->shader->setUniform(MenuOverlay::Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
	}

	glm::ivec2 windowSize = vrlib::Kernel::getInstance()->getWindowSize();

	scissorPush(absPosition.x + 5, absPosition.y, size.x - 15, size.y);
	overlay->drawText(value, absPosition + glm::ivec2(5, 14));
	scissorPop();

	if (focussed && (GetTickCount() / 250) % 2 == 0)
	{
		float offset = overlay->font->textlen(value.substr(0, cursor));
		overlay->drawText("|", absPosition + glm::ivec2(5 + offset-3, 13));
	}


	glDisable(GL_SCISSOR_TEST);
}

bool TextField::click(bool leftButton, const glm::ivec2 & clickPos, int clickCount)
{
	cursor = value.size();
	return true;
}

bool TextField::keyChar(char character)
{
	if (character > 31)
	{
		value = value.substr(0, cursor) + character + value.substr(cursor);
		cursor++;
		if (onChange)
			onChange();
		return true;
	}
	else if (character == 8)
	{
		if (cursor > 0)
		{
			value = value.substr(0, cursor - 1) + value.substr(cursor);
			cursor--;
			if (onChange)
				onChange();
		}
		return true;
	}
	return false;
}

bool TextField::keyUp(int keyCode)
{
	return false;
}

bool TextField::keyDown(int keyCode)
{
	if (keyCode == VK_DELETE && cursor < (int)value.size() - 1)
	{
		value = value.substr(0, cursor) + value.substr(cursor + 1);
		if (onChange)
			onChange();
		return true;
	}

	if (keyCode == VK_LEFT && cursor > 0)
	{
		cursor--;
		return true;
	}
	if (keyCode == VK_RIGHT && cursor < (int)value.size())
	{
		cursor++;
		return true;
	}
	if (keyCode == VK_HOME)
	{
		cursor = 0;
		return true;
	}
	if (keyCode == VK_END)
	{
		cursor = value.size();
		return true;
	}
	return false;
}
