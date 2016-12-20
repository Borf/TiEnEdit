#include "ScrollPanel.h"

#include "../menu/MenuOverlay.h"

ScrollPanel::ScrollPanel(Component * component)
{
	this->component = component;
}

void ScrollPanel::draw(MenuOverlay * overlay)
{
//	overlay->drawRect(glm::vec2(32, 416), glm::vec2(32 + 32, 416 + 32), position, position+size); //background
//	overlay->flushVerts();

	scissorPush(absPosition.x, absPosition.y, size.x, size.y);
	
	component->draw(overlay);

	scissorPop();

}

bool ScrollPanel::click(bool leftButton, const glm::ivec2 & clickPos, int clickCount)
{
	return false;
}

void ScrollPanel::onReposition(Component* parent)
{
	absPosition = position;
	if (parent)
		absPosition += parent->absPosition;
	glm::ivec2 tmpPosition = absPosition;
	absPosition -= scrollOffset;
	component->onReposition(this);
	absPosition = tmpPosition;
}

Component * ScrollPanel::getComponentAt(const glm::ivec2 & pos)
{
	Component* c = component->getComponentAt(pos);
	if (c)
		return c;
	return this;
}


bool ScrollPanel::scroll(float offset)
{
	this->scrollOffset.y -= offset*2;
	glm::ivec2 tmpPosition = absPosition;

	absPosition -= scrollOffset;
	component->onReposition(this);
	absPosition = tmpPosition;
	return true;
}
