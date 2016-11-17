#pragma once

#include "MenuItem.h"



class Menu;

class SubMenuMenuItem : public MenuItem
{
public:
	Menu* menu;
	SubMenuMenuItem(std::string name, Menu* menu);

	bool opened;
};

