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
	node->transform->setGlobalPosition(newPosition);
	editor->cacheSelection = true;
}

void NodeMoveAction::undo(TienEdit * editor)
{
	node->transform->setGlobalPosition(originalPosition);
	editor->cacheSelection = true;
}

void NodeMoveAction::updateNodePointer(vrlib::tien::Node * oldNode, vrlib::tien::Node * newNode)
{
	if (node == oldNode)
		node = newNode;
}
