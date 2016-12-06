#include "Tree.h"
#include "../menu/MenuOverlay.h"

#include <stack>
#include <functional>

const int LINESIZE = 16;
const int TEXTOFFSET = 13;


Tree::Tree()
{
	selectedItem = nullptr;
	selectedIndex = -1;
}

void Tree::draw(MenuOverlay * overlay)
{
	overlay->drawRect(glm::vec2(32, 416), glm::vec2(32 + 32, 416 + 32), position, position + size); //background
	overlay->drawRect(glm::vec2(32, 416), glm::vec2(32 + 32, 416 + 32), glm::vec2(position) + glm::vec2(size.x-scrollBarWidth,0), position+size ); //background
	overlay->flushVerts();





	for (size_t i = 0; i < flatList.size(); i++)
	{
		if (i == selectedIndex)
			overlay->drawRect(glm::vec2(64, 416), glm::vec2(64 + 32, 416 + 32), position + glm::ivec2(0, 3 + LINESIZE * i), position + glm::ivec2(size.x - scrollBarWidth, 3 + LINESIZE * i + LINESIZE)); //selection background

		overlay->flushVerts();
		
		if (flatList[i].hasChildren)
			if (flatList[i].opened)
				overlay->drawRect(glm::vec2(462, 1 + 17 * 1), glm::vec2(462 + 16, 1 + 16 + 17 * 1), position + glm::ivec2(5 + 8 * flatList[i].level, 3 + LINESIZE * i)); // folder opened
			else
				overlay->drawRect(glm::vec2(462, 1 + 17 * 0), glm::vec2(462 + 16, 1 + 16 + 17 * 0), position + glm::ivec2(5 + 8 * flatList[i].level, 3 + LINESIZE * i)); // folder opened

		
		if(flatList[i].icon >= 0)
			overlay->drawRect(glm::vec2(462, 1 + 17* flatList[i].icon), glm::vec2(462 + 16, 1 + 16 + 17* flatList[i].icon), position + glm::ivec2(5 + 8 * flatList[i].level+16, 3 + LINESIZE * i)); // folder opened


		overlay->flushVerts();
		overlay->drawText(flatList[i].text, position + glm::ivec2(5+18+16, 3+ TEXTOFFSET + LINESIZE * i), glm::vec4(1, 1, 1, 1));
	}




}

bool Tree::click(bool leftButton, const glm::ivec2 & clickPos)
{
	int index = (clickPos.y - position.y - 5) / LINESIZE;
	if (index < 0 || index >= (int)flatList.size())
		return true;
	
	if (selectedIndex == index)
	{
		selectedItem = flatList[index].item;
		nodeInfo[selectedItem].opened = !nodeInfo[selectedItem].opened;
		update();

	}
	selectedIndex = index;
	selectedItem = flatList[index].item;





	return true;
}




void Tree::update()
{
	selectedIndex = -1;
	std::function<void(void*, int level)> buildTree;
	flatList.clear();
	buildTree = [&buildTree, this](void* data, int level)
	{
		int childCount = loader->getChildCount(data);

		if (data != nullptr)
		{
			std::string title = "";
			for (int i = 0; i < level; i++)
				title += "  ";

			title += loader->getName(data);

			if (data == selectedItem)
				selectedIndex = flatList.size();

			flatList.push_back(FlatNode(title, childCount > 0, true, level, loader->getIcon(data), data));
		}
		if(data == nullptr || nodeInfo[data].opened)
			for (int i = 0; i < childCount; i++)
				buildTree(loader->getChild(data, i), level + 1);
	};

	buildTree(nullptr, -1);
}