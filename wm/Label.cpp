#include "Label.h"
#include "../menu/MenuOverlay.h"

Label::Label(const std::string & text, const glm::ivec2 & position)
{
	this->text = text;
	this->position = position;
}

void Label::draw(MenuOverlay* overlay)
{
	overlay->drawText(text, position, glm::vec4(1, 1, 1, 1));

}
