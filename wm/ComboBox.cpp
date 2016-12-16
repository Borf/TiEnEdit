#include "ComboBox.h"

#include "../menu/MenuOverlay.h"
#include <VrLib/Kernel.h>

ComboBox::ComboBox(const std::string & value, glm::ivec2 position)
{
	this->value = value;
	this->position = position;
}


void ComboBox::draw(MenuOverlay * overlay)
{
	overlay->drawRect(glm::vec2(128, 328), glm::vec2(128 + 37, 328 + 33), absPosition, absPosition + size); //text background
	overlay->drawRect(glm::vec2(128, 328), glm::vec2(128 + 37, 328 + 33), absPosition + glm::ivec2(size.x - size.y - 10, 0), absPosition + glm::ivec2(size.x, size.y)); //arrow box
	overlay->drawRect(glm::vec2(176, 368), glm::vec2(176 + 16, 368 + 16), absPosition + glm::ivec2(size.x - size.y - 5, 0)); //arrow
	
	if (focussed)
	{
		overlay->drawRect(glm::vec2(128, 328), glm::vec2(128 + 37, 328 + 33), absPosition + glm::ivec2(0, size.y), absPosition + glm::ivec2(size.x, size.y + 16 * values.size() + 5)); //dropdown
		if (overlay->mousePos.y > absPosition.y + size.y && inComponent(overlay->mousePos))
		{
			int index = (int)((overlay->mousePos.y - (absPosition.y + size.y)) / 16);
			overlay->drawRect(glm::vec2(128, 328), glm::vec2(128 + 37, 328 + 33), absPosition + glm::ivec2(0, size.y + 16 * index), absPosition + glm::ivec2(size.x, size.y + 16 * index + 16)); //selection hover background
		}
	}
	overlay->flushVerts();

	glm::ivec2 windowSize = vrlib::Kernel::getInstance()->getWindowSize();

	glEnable(GL_SCISSOR_TEST);
	glScissor(absPosition.x+5, 0, size.x-15, 10000);
	overlay->drawText(value, absPosition + glm::ivec2(5, 14));

	if (focussed)
	{
		for (size_t i = 0; i < values.size(); i++)
		{
			overlay->drawText(values[i], absPosition + glm::ivec2(5, 14 + 20 + 16 * i));
		}
	}
	glDisable(GL_SCISSOR_TEST);

}

bool ComboBox::click(bool leftButton, const glm::ivec2 & clickPos, int clickCount)
{
	if (clickPos.y > absPosition.y + size.y && inComponent(clickPos))
	{
		int index = (clickPos.y - (absPosition.y + size.y)) / 16;
		value = values[index];
		focussed = false;
		if(onChange)
			onChange();
	}
	else if (inComponent(clickPos))
		focussed = !focussed;
	return false;
}

inline bool ComboBox::inComponent(const glm::ivec2 & pos) 
{ 
	if(!focussed)
		return pos.x > absPosition.x && pos.x < absPosition.x + size.x && pos.y > absPosition.y && pos.y < absPosition.y + size.y; 
	else
		return pos.x > absPosition.x && pos.x < absPosition.x + size.x && pos.y > absPosition.y && pos.y < absPosition.y + size.y+16*(int)values.size();
}
