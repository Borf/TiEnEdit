#pragma once

#include "Component.h"

class RenderComponent : public Component
{
	virtual bool click(bool leftButton, const glm::ivec2 &clickPos) {
		return false;
	}

};

