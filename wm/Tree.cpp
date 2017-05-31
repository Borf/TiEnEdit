#include "Tree.h"
#include "../menu/MenuOverlay.h"

#include <VrLib/Font.h>

#include <algorithm>
#include <functional>

#include <Windows.h>


const int LINESIZE = 16;
const int TEXTOFFSET = 13;

template<class T>
Tree<T>::Tree()
{
	scrollOffset = 0;
	dragging = false;
	this->focusable = true;
}

template<class T>
void Tree<T>::draw(MenuOverlay * overlay)
{
	overlay->drawRect(glm::vec2(32, 416), glm::vec2(32 + 32, 416 + 32), absPosition, absPosition + size); //background
	overlay->drawRect(glm::vec2(32, 416), glm::vec2(32 + 32, 416 + 32), glm::vec2(absPosition) + glm::vec2(size.x-scrollBarWidth,0), position+size ); //background
	overlay->flushVerts();

	scissorPush(absPosition.x + 5, absPosition.y, size.x - 15, size.y);



	for (size_t i = 0; i < flatList.size(); i++)
	{
		if (std::find(std::begin(selectedIndices), std::end(selectedIndices), i) != std::end(selectedIndices))
			overlay->drawRect(glm::vec2(64, 416), glm::vec2(64 + 32, 416 + 32), absPosition + glm::ivec2(0, -scrollOffset + 3 + LINESIZE * i), absPosition + glm::ivec2(size.x - scrollBarWidth, -scrollOffset + 3 + LINESIZE * i + LINESIZE)); //selection background

		overlay->flushVerts();
		
		if (flatList[i].hasChildren)
			if (flatList[i].opened)
				overlay->drawRect(glm::vec2(462, 1 + 17 * 1), glm::vec2(462 + 16, 1 + 16 + 17 * 1), absPosition + glm::ivec2(5 + 8 * flatList[i].level, -scrollOffset + 3 + LINESIZE * i)); // folder opened
			else
				overlay->drawRect(glm::vec2(462, 1 + 17 * 0), glm::vec2(462 + 16, 1 + 16 + 17 * 0), absPosition + glm::ivec2(5 + 8 * flatList[i].level, -scrollOffset + 3 + LINESIZE * i)); // folder opened

		
		if(flatList[i].icon >= 0)
			overlay->drawRect(glm::vec2(462, 1 + 17* flatList[i].icon), glm::vec2(462 + 16, 1 + 16 + 17* flatList[i].icon), absPosition + glm::ivec2(5 + 8 * flatList[i].level+16, -scrollOffset + 3 + LINESIZE * i)); // folder opened


		overlay->flushVerts();
		overlay->drawText(flatList[i].text, absPosition + glm::ivec2(5+18+16, -scrollOffset + 3+ TEXTOFFSET + LINESIZE * i), (!selectedIndices.empty() && selectedIndices[0] == i) ? glm::vec4(1,1,0,1) : glm::vec4(1, 1, 1, 1));
	}


	if (dragging && dragIndex >= 0 && dragIndex < (int)flatList.size())
	{
		float textlen = overlay->font->textlen(flatList[dragIndex].text);
		overlay->drawRect(glm::vec2(64, 416), glm::vec2(64 + 32, 416 + 32), dragPos, dragPos + glm::ivec2(textlen+10, LINESIZE)); //selection background
		overlay->flushVerts();
		overlay->drawText(flatList[dragIndex].text, dragPos + glm::ivec2(5, TEXTOFFSET), glm::vec4(1, 1, 1, 1));

	}

	scissorPop();


}

template<class T>
bool Tree<T>::click(bool leftButton, const glm::ivec2 & clickPos, int clickCount)
{
	dragging = false;
	int index = (int)((clickPos.y - absPosition.y - 5 + scrollOffset) / LINESIZE);
	if (index < 0 || index >= (int)flatList.size())
	{
		selectedIndices.clear();
		selectedItems.clear();
		if (!leftButton)
			rightClickItem();
		return true;
	}
	bool shift = ((GetKeyState(VK_LSHIFT) | GetKeyState(VK_RSHIFT)) & 0x80) != 0;
	bool ctrl = ((GetKeyState(VK_LCONTROL) | GetKeyState(VK_RCONTROL)) & 0x80) != 0;

	if (leftButton)
	{
		if (std::find(std::begin(selectedIndices), std::end(selectedIndices), index) != std::end(selectedIndices))
		{
			if (clickCount == 2)
			{
				if (flatList[index].hasChildren)
				{
					nodeInfo[flatList[index].item].opened = !nodeInfo[flatList[index].item].opened;
					update();
				}
				else
					if(doubleClickItem)
						doubleClickItem();
			}
			else
			{
				if (ctrl)
				{
					int i = std::distance(std::begin(selectedIndices), std::find(std::begin(selectedIndices), std::end(selectedIndices), index));
					selectedIndices.erase(selectedIndices.begin() + i);
					selectedItems.erase(selectedItems.begin() + i);
				}
				else
				{
					int i = std::distance(std::begin(selectedIndices), std::find(std::begin(selectedIndices), std::end(selectedIndices), index));
					std::swap(selectedIndices[0], selectedIndices[i]);
					std::swap(selectedItems[0], selectedItems[i]);
				}
			}

		}
		else
		{

			selectedItems.clear();
			if(!ctrl && !shift)
				selectedIndices.clear();

			if(ctrl || !shift)
				selectedIndices.push_back(index);

			if (shift && !selectedIndices.empty())
			{
				int lastIndex = selectedIndices.back();
				if (index < lastIndex)
					lastIndex--;
				for (int i = lastIndex + 1; i != index; i+=index > lastIndex?1:-1)
					selectedIndices.push_back(i);
				selectedIndices.push_back(index);
				std::unique(selectedIndices.begin(), selectedIndices.end());
			}

			for(auto i : selectedIndices)
				selectedItems.push_back(flatList[i].item);
			if (selectItem)
				selectItem();
		}
	}
	else
	{
		if (rightClickItem)
			rightClickItem();
		else
		{
			selectedItems.clear();
			selectedIndices.clear();
			selectedIndices.push_back(index);
			selectedItems.push_back(flatList[index].item);
		}
	}



	return true;
}

template<class T>
bool Tree<T>::mouseDown(bool leftButton, const glm::ivec2 & mousePos)
{
	return false;
}

template<class T>
bool Tree<T>::mouseUp(bool leftButton, const glm::ivec2 & mousePos)
{
	dragging = false;
	return false;
}

template<class T>
bool Tree<T>::mouseDrag(bool leftButton, const glm::ivec2 & startPos, const glm::ivec2 & mousePos, const glm::ivec2 & lastMousePos)
{
	if (!dragging)
	{
		dragIndex = (int)((startPos.y - absPosition.y - 5 + scrollOffset) / LINESIZE);
		if (std::find(selectedIndices.begin(), selectedIndices.end(), dragIndex) == selectedIndices.end() && dragIndex < (int)flatList.size())
		{
			selectedIndices.clear();
			selectedIndices.push_back(dragIndex);
			selectedItems = { flatList[dragIndex].item };
		}

	}
	dragging = true;
	dragPos = mousePos;
	return true;
}

template<class T>
bool Tree<T>::mouseFinishDrag(bool leftButton, const glm::ivec2 & startPos, const glm::ivec2 & mousePos)
{
	if (!dragging)
		return false;
	dragging = false;
	if (!inComponent(mousePos))
		return false;
	int indexFrom = dragIndex;
	int indexTo = (int)((mousePos.y - absPosition.y - 5 + scrollOffset) / LINESIZE);

	T to = nullptr;
	if (indexTo >= 0 && indexTo < (int)flatList.size())
		to = flatList[indexTo].item;


	if (std::find(selectedItems.begin(), selectedItems.end(), to) != selectedItems.end())
		return true;



	if (dragItem)
		dragItem(selectedItems, to);
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