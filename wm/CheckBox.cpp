#include "CheckBox.h"

#include "../menu/MenuOverlay.h"
#include <VrLib/Kernel.h>

CheckBox::CheckBox(bool value, glm::ivec2 position)
{
	this->value = value;
	this->position = position;
	this->size = glm::ivec2(15, 15);
	this->focusable = true;
}


void CheckBox::draw(MenuOverlay * overlay)
{
	overlay->drawRect(glm::vec2(16, 368), glm::vec2(16 + 15, 368 + 15), absPosition + glm::ivec2(0, 2));
	overlay->flushVerts();
	if (focussed)
	{
		overlay->shader->setUniform(MenuOverlay::Uniforms::colorMult, glm::vec4(0.1f, 0.1f, 1, 1)); //TODO
		overlay->drawRect(glm::vec2(16, 368), glm::vec2(16 + 15, 368 + 15), absPosition + glm::ivec2(0, 2));
		overlay->flushVerts();
		overlay->shader->setUniform(MenuOverlay::Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
	}
	if(value)
		overlay->drawRect(glm::vec2(0, 385), glm::vec2(15, 385+15), absPosition + glm::ivec2(0, 2));


	overlay->flushVerts();
}

bool CheckBox::click(bool leftButton, const glm::ivec2 & clickPos, int clickCount)
{
	this->value = !this->value;
	if(onChange)
		onChange();
	return true;
}

bool CheckBox::keyChar(char character)
{
	if (character == ' ')
	{
		value = !value;
		return true;
	}
	return false;
}
