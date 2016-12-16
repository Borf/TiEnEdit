#include "Panel.h"

#include "../menu/MenuOverlay.h"

void Panel::draw(MenuOverlay * overlay)
{
	overlay->drawRect(glm::vec2(32, 416), glm::vec2(32 + 32, 416 + 32), position, position+size); //menubar
	overlay->flushVerts();

	for (auto c : components)
		if (!c->focussed)
			c->draw(overlay);
	for (auto c : components)
		if (c->focussed)
			c->draw(overlay);

}

bool Panel::click(bool leftButton, const glm::ivec2 & clickPos, int clickCount)
{
	for (auto c : components)
		if (c->inComponent(clickPos))
			return c->click(leftButton, clickPos, clickCount);
	return false;
}

void Panel::onReposition(Component* parent)
{
	absPosition = position;
	if (parent)
		absPosition += parent->absPosition;
	for (auto c : components)
	{
	}


	for (auto c : components)
	{
		c->onReposition(this);
	}
}

Component * Panel::getComponentAt(const glm::ivec2 & pos)
{
	for (auto c : components)
		if (c->inComponent(pos))
			return c->getComponentAt(pos);
	return nullptr;
}
