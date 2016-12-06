#include "Panel.h"

#include "../menu/MenuOverlay.h"

void Panel::draw(MenuOverlay * overlay)
{
	overlay->drawRect(glm::vec2(32, 416), glm::vec2(32 + 32, 416 + 32), position, position+size); //menubar

	overlay->flushVerts();
}

bool Panel::click(bool leftButton, const glm::ivec2 & clickPos)
{
	return false;
}
