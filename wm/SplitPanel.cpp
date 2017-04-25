#include "SplitPanel.h"


SplitPanel::SplitPanel(Alignment alignment)
{
	this->alignment = alignment;
}

void SplitPanel::addPanel(Component * panel)
{
	components.push_back(panel);
	sizes.push_back(100);
}


void SplitPanel::onReposition(Component* parent)
{
	absPosition = position;
	if (parent)
		absPosition = parent->absPosition + position;

	glm::ivec2 pos = glm::vec2(0,0);
	for (size_t i = 0; i < components.size(); i++)
	{
		components[i]->position = pos;
		if (alignment == Alignment::HORIZONTAL)
		{
			components[i]->size = glm::ivec2(sizes[i], size.y);
			pos.x += sizes[i];
		}
		else
		{
			components[i]->size = glm::ivec2(size.x, sizes[i]);
			pos.y += sizes[i];
		}
	}
	if(alignment == Alignment::HORIZONTAL)
		components[components.size() - 1]->size.x += (size.x - pos.x); //make last one as large as possible
	else
		components[components.size() - 1]->size.y += (size.y - pos.y); //make last one as large as possible

	for (auto p : components)
		p->onReposition(this);
}

void SplitPanel::draw(MenuOverlay * overlay)
{
	for (auto p : components)
		p->draw(overlay);
}

bool SplitPanel::click(bool leftButton, const glm::ivec2 & clickPos, int clickCount)
{
	if (!inComponent(clickPos))
		return false;

	for (auto p : components)
		if (p->inComponent(clickPos))
			return p->click(leftButton, clickPos, clickCount);
	return false;
}

bool SplitPanel::mouseUp(bool leftButton, const glm::ivec2 & clickPos)
{
	if (!inComponent(clickPos))
		return false;

	for (auto p : components)
		if (p->inComponent(clickPos))
			return p->mouseUp(leftButton, clickPos);
	return false;
}

