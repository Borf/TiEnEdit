#include "SubMenuMenuItem.h"

SubMenuMenuItem::SubMenuMenuItem(std::string name, Menu* menu) : MenuItem(name)
{
	this->menu = menu;
	opened = false;
}
