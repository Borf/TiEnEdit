#include "SplitPanel.h"


SplitPanel::SplitPanel()
{
}

void SplitPanel::addPanel(Component * panel)
{
	panels.push_back(panel);
	sizes.push_back(100);
}


void SplitPanel::onReposition()
{
	glm::ivec2 pos = position;
	for (size_t i = 0; i < panels.size(); i++)
	{
		panels[i]->position = pos;
		panels[i]->size = glm::ivec2(sizes[i], size.y);
		pos.x += sizes[i];
	}
	panels[panels.size() - 1]->size.x += (size.x - pos.x); //make last one as large as possible
	for (auto p : panels)
		p->onReposition();
}