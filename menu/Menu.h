#pragma once

#include <string>
#include <vector>
#include <functional>
#include <VrLib/json.hpp>


class MenuItem;

class Menu
{
public:
	Menu(const json &data);
	void setAction(std::string path, std::function<void() > callback);
	void linkToggle(std::string path, bool* linkBool);
	void setToggleValue(std::string path, bool value);
	MenuItem* getItem(std::string path);

	void foreach(std::function<void(MenuItem*)> callback);
	void setMenu(std::string menuLoc, MenuItem* menuItem);
	std::vector<MenuItem*> menuItems;
};

