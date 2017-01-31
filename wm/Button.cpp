#include "Button.h"

#include "../menu/MenuOverlay.h"
#include <VrLib/Kernel.h>
#include <VrLib/Font.h>

Button::Button(const std::string &text, glm::ivec2 position)
{
	this->text = text;
	this->position = position;
	this->size = glm::ivec2(50, 25);
}


void Button::draw(MenuOverlay * overlay)
{
	overlay->drawRect(glm::vec2(128, 328), glm::vec2(128 + 37, 328 + 33), absPosition, absPosition + size); //button
	overlay->flushVerts();

	float len = overlay->font->textlen(text);

	overlay->drawText(text, absPosition + glm::ivec2((size.x - len) / 2, 14));

}

bool Button::click(bool leftButton, const glm::ivec2 & clickPos, int clickCount)
{
	if(onClick)
		onClick();
	return true;
}
