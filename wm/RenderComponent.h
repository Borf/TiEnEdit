#pragma once

#include "Component.h"

class TienEdit;

class RenderComponent : public Component
{
	TienEdit* editor;
public:
	RenderComponent(TienEdit* editor);

	virtual bool click(bool leftButton, const glm::ivec2 &clickPos, int clickCount) {
		return false;
	}


	virtual void handleDrag(DragProperties* properties) override;


};

