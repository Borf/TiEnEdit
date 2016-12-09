#include "Component.h"

void Component::onReposition(Component * parent)
{
	this->absPosition = parent->absPosition + position;
}
