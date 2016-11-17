#pragma once

#include "MenuItem.h"
#include <functional>

class ActionMenuItem : public MenuItem
{
public:
	std::function<void()>	callback;

	ActionMenuItem(std::string name) : MenuItem(name)
	{
		callback = nullptr;
	}
};
