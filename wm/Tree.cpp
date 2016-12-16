#include "Tree.h"
#include "../menu/MenuOverlay.h"

#include <algorithm>
#include <functional>

const int LINESIZE = 16;
const int TEXTOFFSET = 13;

template<class T>
Tree<T>::Tree()
{
	scrollOffset = 0;
}

template<class T>
void Tree<T>::draw(MenuOverlay * overlay)
{
	overlay->drawRect(glm::vec2(32, 416), glm::vec2(32 + 32, 416 + 32), position, position + size); //background
	overlay->drawRect(glm::vec2(32, 416), glm::vec2(32 + 32, 416 + 32), glm::vec2(position) + glm::vec2(size.x-scrollBarWidth,0), position+size ); //background
	overlay->flushVerts();

	//TODO: clip



	for (size_t i = 0; i < flatList.size(); i++)
	{
		if (std::find(std::begin(selectedIndices), std::end(selectedIndices), i) != std::end(selectedIndices))
			overlay->drawRect(glm::vec2(64, 416), glm::vec2(64 + 32, 416 + 32), position + glm::ivec2(0, -scrollOffset + 3 + LINESIZE * i), position + glm::ivec2(size.x - scrollBarWidth, -scrollOffset + 3 + LINESIZE * i + LINESIZE)); //selection background

		overlay->flushVerts();
		
		if (flatList[i].hasChildren)
			if (flatList[i].opened)
				overlay->drawRect(glm::vec2(462, 1 + 17 * 1), glm::vec2(462 + 16, 1 + 16 + 17 * 1), position + glm::ivec2(5 + 8 * flatList[i].level, -scrollOffset + 3 + LINESIZE * i)); // folder opened
			else
				overlay->drawRect(glm::vec2(462, 1 + 17 * 0), glm::vec2(462 + 16, 1 + 16 + 17 * 0), position + glm::ivec2(5 + 8 * flatList[i].level, -scrollOffset + 3 + LINESIZE * i)); // folder opened

		
		if(flatList[i].icon >= 0)
			overlay->drawRect(glm::vec2(462, 1 + 17* flatList[i].icon), glm::vec2(462 + 16, 1 + 16 + 17* flatList[i].icon), position + glm::ivec2(5 + 8 * flatList[i].level+16, -scrollOffset + 3 + LINESIZE * i)); // folder opened


		overlay->flushVerts();
		overlay->drawText(flatList[i].text, position + glm::ivec2(5+18+16, -scrollOffset + 3+ TEXTOFFSET + LINESIZE * i), glm::vec4(1, 1, 1, 1));
	}




}

template<class T>
bool Tree<T>::click(bool leftButton, const glm::ivec2 & clickPos, int clickCount)
{
	int index = (clickPos.y - position.y - 5 + scrollOffset) / LINESIZE;
	if (index < 0 || index >= (int)flatList.size())
	{
		selectedIndices.clear();
		selectedItems.clear();
		if (!leftButton)
			rightClickItem();
		return true;
	}
	
	if (leftButton)
	{
		if (std::find(std::begin(selectedIndices), std::end(selectedIndices), index) != std::end(selectedIndices))
		{
			if (clickCount == 2)
			{
				if(doubleClickItem)
					doubleClickItem();
			}

			/*selectedIndices.clear();
			selectedItems.push_back(flatList[index].item);
			nodeInfo[selectedItem].opened = !nodeInfo[selectedItem].opened;
			update();*/
		}
		else
		{
			selectedItems.clear();
			selectedIndices.clear();
			selectedIndices.push_back(index);
			selectedItems.push_back(flatList[index].item);
			if (selectItem)
				selectItem();
		}
	}
	else
	{
		selectedItems.clear();
		selectedIndices.clear();
		selectedIndices.push_back(index);
		selectedItems.push_back(flatList[index].item);
		if (rightClickItem)
			rightClickItem();
	}



	return true;
}



template<class T>
void Tree<T>::update()
{
	selectedIndices.clear();
	std::function<void(vrlib::tien::Node*, int level)> buildTree;
	flatList.clear();
	buildTree = [&buildTree, this](vrlib::tien::Node* data, int level)
	{
		int childCount = loader->getChildCount(data);

		if (data != nullptr)
		{
			std::string title = "";
			for (int i = 0; i < level; i++)
				title += "  ";

			title += loader->getName(data);

			if (std::find(std::begin(selectedItems), std::end(selectedItems), data) != std::end(selectedItems))
				selectedIndices.push_back(flatList.size());

			flatList.push_back(FlatNode(title, childCount > 0, true, level, loader->getIcon(data), data));
		}
		if(data == nullptr || nodeInfo[data].opened)
			for (int i = 0; i < childCount; i++)
				buildTree(loader->getChild(data, i), level + 1);
	};

	buildTree(nullptr, -1);
}

template<class T>
bool Tree<T>::scroll(float offset)
{
	this->scrollOffset -= offset * 3;
	scrollOffset = glm::clamp(scrollOffset, 0.0f, (float)(LINESIZE * flatList.size() - size.y + 100));
	return true;
}


//TODO: move this?
#include <VrLib/tien/Node.h>
template class Tree<vrlib::tien::Node*>;