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

class NodeMoveAction : public Action
{
	vrlib::tien::Node* node;
	glm::vec3 originalPosition;
	glm::vec3 newPosition;
public:
	NodeMoveAction(vrlib::tien::Node* node, const glm::vec3 &originalPosition, const glm::vec3 &newPosition);

	virtual void perform(TienEdit * editor) override;
	virtual void undo(TienEdit * editor) override;
};