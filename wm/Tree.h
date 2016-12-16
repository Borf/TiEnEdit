#pragma once

#include "Component.h"
#include <string>
#include <vector>
#include <map>
#include <functional>

template<class T>
class Tree : public Component
{
	const static int scrollBarWidth = 16;

	class NodeInfo
	{
	public:
		bool opened = false;
	};
	std::map<T, NodeInfo> nodeInfo;


	class FlatNode
	{
	public:
		std::string text;
		bool hasChildren;
		bool opened;
		T item;
		int level;
		int icon;

		FlatNode(const std::string &text, bool hasChildren, bool opened, int level, int icon, T item)
		{
			this->text = text;
			this->hasChildren = hasChildren;
			this->opened = opened;
			this->item = item;
			this->level = level;
			this->icon = icon;
		}
	};
	std::vector<FlatNode> flatList;
	std::vector<int> selectedIndices;
	float scrollOffset;
	bool dragging;
	glm::ivec2 dragPos;
	int dragIndex;


public:
	class TreeLoader
	{
	public:
		virtual std::string getName(T data) = 0;
		virtual int getChildCount(T data) = 0;
		virtual T getChild(T data, int index) = 0;
		virtual int getIcon(T data) = 0;
	};
	std::vector<T> selectedItems;


	std::function<void()> selectItem;
	std::function<void()> doubleClickItem;
	std::function<void()> rightClickItem;
	std::function<void(T from, T to)> dragItem;


	Tree();
	void draw(MenuOverlay* overlay) override;
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos, int clickCount) override;
	virtual bool mouseDown(bool leftButton, const glm::ivec2 &mousePos)  override;
	virtual bool mouseUp(bool leftButton, const glm::ivec2 &mousePos)  override;
	virtual bool mouseDrag(bool leftButton, const glm::ivec2 &startPos, const glm::ivec2 &mousePos) override;
	virtual bool mouseFinishDrag(bool leftButton, const glm::ivec2 &startPos, const glm::ivec2 &mousePos) override;
	virtual bool scroll(float offset) override;
	void update();

	TreeLoader* loader;
};

