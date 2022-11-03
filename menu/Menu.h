#pragma once

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <VrLib/json.hpp>


class MenuItem;

class Menu
{
	Menu(const nlohmann::json &data);
	static std::map<std::string, Menu*> loadedMenus;
public:
	static Menu* load(const std::string &filename);

	void setAction(std::string path, std::function<void() > callback);
	void linkToggle(std::string path, bool* linkBool);
	void setToggleValue(std::string path, bool value);
	MenuItem* getItem(std::string path);

	void foreach(std::function<void(MenuItem*)> callback);
	void setMenu(std::string menuLoc, MenuItem* menuItem);
	std::vector<MenuItem*> menuItems;
};

