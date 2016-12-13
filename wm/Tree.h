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
	std::function<void()> rightClickItem;


	Tree();
	void draw(MenuOverlay* overlay) override;
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos) override;

	void update();

	TreeLoader* loader;
};

