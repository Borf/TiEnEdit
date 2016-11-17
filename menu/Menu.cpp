#include "Menu.h"

#include "SubMenuMenuItem.h"
#include "MenuItem.h"
#include "ToggleMenuItem.h"
#include "ActionMenuItem.h"

#include <vrlib/json.h>
#include <vrlib/Log.h>

using vrlib::logger; using vrlib::Log;


Menu::Menu(const vrlib::json::Value &data)
{
	for (size_t i = 0; i < data.size(); i++)
	{
		MenuItem* subItem = NULL;

		if (data[i]["type"].asString() == "menu")
		{
			subItem = new SubMenuMenuItem(data[i]["name"].asString(), new Menu(data[i]["subitems"]));
		}
		else if (data[i]["type"].asString() == "item")
		{
			subItem = new ActionMenuItem(data[i]["name"].asString());
		}
		else if (data[i]["type"].asString() == "toggle")
		{
			subItem = new ToggleMenuItem(data[i]["name"].asString(), data[i]["initial"].asBool());
		}
		else
			logger<< "Unknown menu type: " << data[i]["type"].asString() << Log::newline;

		if (subItem)
		{
			menuItems.push_back(subItem);
		}
	}
}

void Menu::setAction(std::string path, std::function<void() > callback)
{
	std::string firstPart = path;
	if (firstPart.find("/") != std::string::npos)
	{
		firstPart = firstPart.substr(0, firstPart.find("/"));
	}

	for (size_t i = 0; i < menuItems.size(); i++)
	{
		if (menuItems[i]->name == firstPart)
		{
			SubMenuMenuItem* subMenu = dynamic_cast<SubMenuMenuItem*>(menuItems[i]);
			if (subMenu)
				subMenu->menu->setAction(path.substr(firstPart.size() + 1), callback);

			ActionMenuItem* item = dynamic_cast<ActionMenuItem*>(menuItems[i]);
			if (item)
				item->callback = callback;
		}
	}
}

void Menu::linkToggle(std::string path, bool* linkBool)
{
	std::string firstPart = path;
	if (firstPart.find("/") != std::string::npos)
	{
		firstPart = firstPart.substr(0, firstPart.find("/"));
	}

	for (size_t i = 0; i < menuItems.size(); i++)
	{
		if (menuItems[i]->name == firstPart)
		{
			SubMenuMenuItem* subMenu = dynamic_cast<SubMenuMenuItem*>(menuItems[i]);
			if (subMenu)
				subMenu->menu->linkToggle(path.substr(firstPart.size() + 1), linkBool);

			ToggleMenuItem* item = dynamic_cast<ToggleMenuItem*>(menuItems[i]);
			if (item)
				item->linkToggle(linkBool);
		}
	}
}

void Menu::setToggleValue(std::string path, bool value)
{
	std::string firstPart = path;
	if (firstPart.find("/") != std::string::npos)
	{
		firstPart = firstPart.substr(0, firstPart.find("/"));
	}

	for (size_t i = 0; i < menuItems.size(); i++)
	{
		if (menuItems[i]->name == firstPart)
		{
			SubMenuMenuItem* subMenu = dynamic_cast<SubMenuMenuItem*>(menuItems[i]);
			if (subMenu)
				subMenu->menu->setToggleValue(path.substr(firstPart.size() + 1), value);

			ToggleMenuItem* item = dynamic_cast<ToggleMenuItem*>(menuItems[i]);
			if (item)
			{
				if (item->getValue() != value)
					item->toggle();
			}
		}
	}
}

MenuItem* Menu::getItem(std::string path)
{
	std::string firstPart = path;
	if (firstPart.find("/") != std::string::npos)
	{
		firstPart = firstPart.substr(0, firstPart.find("/"));
	}

	for (size_t i = 0; i < menuItems.size(); i++)
	{
		if (menuItems[i]->name == firstPart)
		{
			if (menuItems[i]->name == path)
				return menuItems[i];
			SubMenuMenuItem* subMenu = dynamic_cast<SubMenuMenuItem*>(menuItems[i]);
			if (subMenu)
				return subMenu->menu->getItem(path.substr(firstPart.size() + 1));

			return menuItems[i];
		}
	}
	return NULL;
}

void Menu::foreach(std::function<void(MenuItem*)> callback)
{
	for (size_t i = 0; i < menuItems.size(); i++)
	{
		SubMenuMenuItem* subMenu = dynamic_cast<SubMenuMenuItem*>(menuItems[i]);
		if (subMenu)
			subMenu->menu->foreach(callback);
		else
			callback(menuItems[i]);
	}
}

void Menu::setMenu(std::string menuLoc, MenuItem* menuItem)
{
	if (menuLoc == "")
	{
		menuItems.push_back(menuItem);
		return;
	}

	std::string first = menuLoc;
	if (first.find("/") != std::string::npos)
		first = first.substr(0, first.find("/"));

	for (size_t i = 0; i < menuItems.size(); i++)
	{
		if (menuItems[i]->name == first)
		{
			((SubMenuMenuItem*)menuItems[i])->menu->setMenu(menuLoc.substr(first.length() == menuLoc.length() ? first.length() : first.length() + 1), menuItem);
			return;
		}
	}
	
	Menu* menu = new Menu(vrlib::json::Value(vrlib::json::Type::arrayValue));
	menu->setMenu(menuLoc.substr(first.length() == menuLoc.length() ? first.length() : first.length() + 1), menuItem);
	menuItems.push_back(new SubMenuMenuItem(first, menu));
}

