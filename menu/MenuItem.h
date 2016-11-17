#pragma once

#include <string>


class MenuItem
{
public:
	std::string name;
	std::string description;
	bool enabled;
	MenuItem(std::string name);
	virtual ~MenuItem() {}
};
