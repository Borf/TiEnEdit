#include "TextField.h"

#include "../menu/MenuOverlay.h"
#include <VrLib/Kernel.h>
#include <VrLib/Font.h>

#include <Windows.h> // TODO: keycodes

TextField::TextField(const std::string & value, glm::ivec2 position)
{
	this->value = value;
	this->absPosition = this->position = position;
	this->cursor = 0;
	this->selectionEnd = 0;
	this->readonly = false;
	this->focusable = true;
}


void TextField::draw(MenuOverlay * overlay)
{
	if (!font)
		font = overlay->font;
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
	float offset = 0;
	if (focussed)
	{
		offset = overlay->font->textlen(value.substr(0, cursor)) - offsetX;
		while (offset < 0)
		{
			offsetX -= 40;
			offset = overlay->font->textlen(value.substr(0, cursor)) - offsetX;
		}
		while (offset > size.x-10)
		{
			offsetX += 40;
			offset = overlay->font->textlen(value.substr(0, cursor)) - offsetX;
		}
	}

	if (cursor != selectionEnd && focussed)
	{
		float selectionEndPos = overlay->font->textlen(value.substr(0, selectionEnd)) - offsetX;

		overlay->drawRect(glm::vec2(114, 503), glm::vec2(114 + 7, 503 + 7), absPosition + glm::ivec2(5 + offset, 2), absPosition + glm::ivec2(5 + selectionEndPos, 16));
		overlay->flushVerts();
	}


	overlay->drawText(value, absPosition + glm::ivec2(5 - offsetX, 14));

	if (focussed && (GetTickCount() / 250) % 2 == 0)
	{
		overlay->drawText("|", absPosition + glm::ivec2(5 + offset - 3, 13));
	}

	scissorPop();
}

bool TextField::click(bool leftButton, const glm::ivec2 & clickPos, int clickCount)
{
	if (clickCount == 2)
	{
		cursor = value.size();
		selectionEnd = 0;
		return true;
	}
	return true;
}

bool TextField::mouseDown(bool leftButton, const glm::ivec2 & clickPos)
{
	float clickPosX = clickPos.x + offsetX - absPosition.x;
	cursor = selectionEnd = value.size();

	for (size_t i = 0; i < value.size(); i++)
	{
		float textLen = font->textlen(value.substr(0, i));
		if (clickPosX <= textLen)
		{
			cursor = i-1;
			selectionEnd = cursor;
			break;
		}
	}

	return true;
}

bool TextField::keyChar(char character)
{
	if (readonly)
		return true;
	if (character > 31)
	{
		if (cursor != selectionEnd && !value.empty())
		{
			value = value.substr(0, glm::min(cursor, selectionEnd)) + value.substr(glm::max(cursor, selectionEnd));
			cursor = glm::min(cursor, selectionEnd);
			selectionEnd = cursor;
		}
		std::string pre = "";
		if (cursor > 0)
			pre = value.substr(0, cursor);
		value = pre + character + value.substr(cursor);
		cursor++;
		selectionEnd = cursor;
		if (onChange)
			onChange();
		return true;
	}
	else if (character == 8) //backspace
	{
		if (cursor != selectionEnd && !value.empty())
		{
			value = value.substr(0, glm::min(cursor, selectionEnd)) + value.substr(glm::max(cursor, selectionEnd));
			cursor = glm::min(cursor, selectionEnd);
			selectionEnd = cursor;
			if (onChange)
				onChange();
			return true;
		}


		if (cursor > 0 && !value.empty())
		{
			value = value.substr(0, cursor - 1) + value.substr(cursor);
			cursor--;
			selectionEnd = cursor;
			if (onChange)
				onChange();
		}
		return true;
	}
	return false;
}

bool TextField::keyUp(int keyCode)
{
	if (readonly)
		return true;

	if (keyCode == VK_DELETE || keyCode == VK_LEFT || keyCode == VK_RIGHT || keyCode == VK_HOME || keyCode == VK_END)
		return true;
	if ((keyCode >= 'a' && keyCode <= 'z') || (keyCode >= 'A' && keyCode <= 'Z') || (keyCode >= '0' && keyCode <= '9'))
		return true;

	return false;
}

bool TextField::keyDown(int keyCode)
{
	if (keyCode == VK_DELETE && cursor != selectionEnd && !readonly)
	{
		value = value.substr(0, glm::min(cursor, selectionEnd)) + value.substr(glm::max(cursor, selectionEnd));
		cursor = glm::min(cursor, selectionEnd);
		selectionEnd = cursor;
		if (onChange)
			onChange();
		return true;
	}

	if (keyCode == VK_DELETE && cursor < (int)value.size() && !readonly)
	{
		value = value.substr(0, cursor) + value.substr(cursor + 1);
		selectionEnd = cursor;
		if (onChange)
			onChange();
		return true;
	}

	if (keyCode == VK_LEFT && cursor > 0)
	{
		cursor--;
		if ((GetKeyState(VK_SHIFT) & 0x80) == 0)
			selectionEnd = cursor;
		return true;
	}
	if (keyCode == VK_RIGHT && cursor < (int)value.size())
	{
		cursor++;
		if ((GetKeyState(VK_SHIFT) & 0x80) == 0)
			selectionEnd = cursor;
		return true;
	}
	if (keyCode == VK_HOME)
	{
		cursor = 0;
		if ((GetKeyState(VK_SHIFT) & 0x80) == 0)
			selectionEnd = cursor;
		return true;
	}
	if (keyCode == VK_END)
	{
		cursor = value.size();
		if ((GetKeyState(VK_SHIFT) & 0x80) == 0)
			selectionEnd = cursor;
		return true;
	}
	return false;
}

void TextField::focus()
{
	this->cursor = 0;
	this->selectionEnd = value.size();
}

bool TextField::mouseDrag(bool leftButton, const glm::ivec2 & startPos, const glm::ivec2 & mousePos, const glm::ivec2 & lastMousePos)
{
	float clickPosX = mousePos.x + offsetX - absPosition.x;
	cursor = value.size();

	for (size_t i = 0; i < value.size(); i++)
	{
		float textLen = font->textlen(value.substr(0, i));
		if (clickPosX <= textLen)
		{
			cursor = i-1;
			break;
		}
	}
	return true;
}

