#pragma once

#include "Action.h"
#include <glm/glm.hpp>

namespace vrlib
{
	namespace tien
	{
		class Node;
	}
}

class NodeScaleAction : public Action
{
public:
	vrlib::tien::Node* node;
	glm::vec3 originalScale;
	glm::vec3 originalPosition;
	glm::vec3 newScale;
	glm::vec3 newPosition;


	NodeScaleAction(vrlib::tien::Node* node);

	virtual void perform(TienEdit * editor) override;
	virtual void undo(TienEdit * editor) override;
};