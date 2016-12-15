#include "NodeMoveAction.h"
#include "../TienEdit.h"

#include <VrLib/tien/Node.h>
#include <VrLib/tien/components/Transform.h>

NodeMoveAction::NodeMoveAction(vrlib::tien::Node* node, const glm::vec3 &originalPosition, const glm::vec3 &newPosition)
{
	this->node = node;
	this->originalPosition = originalPosition;
	this->newPosition = newPosition;
}

void NodeMoveAction::perform(TienEdit * editor)
{
	node->transform->position = newPosition;
	editor->cacheSelection = true;
}

void NodeMoveAction::undo(TienEdit * editor)
{
	node->transform->position = originalPosition;
	editor->cacheSelection = true;
}
