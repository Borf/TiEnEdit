#pragma once

#include "Component.h"
#include <string>
#include <vector>
#include <map>
#include <functional>

class Tree : public Component
{
	const static int scrollBarWidth = 16;

	class NodeInfo
	{
	public:
		bool opened = false;
	};
	std::map<void*, NodeInfo> nodeInfo;


	class FlatNode
	{
	public:
		std::string text;
		bool hasChildren;
		bool opened;
		void* item;
		int level;
		int icon;

		FlatNode(const std::string &text, bool hasChildren, bool opened, int level, int icon, void* item)
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
	int selectedIndex;


public:

	class TreeLoader
	{
	public:
		virtual std::string getName(void* data) = 0;
		virtual int getChildCount(void* data) = 0;
		virtual void* getChild(void* data, int index) = 0;
		virtual int getIcon(void* data) = 0;
	};
	void* selectedItem;


	std::function<void()> selectItem;
	std::function<void()> rightClickItem;


	Tree();
	void draw(MenuOverlay* overlay) override;
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos) override;

	void update();

	TreeLoader* loader;
};

