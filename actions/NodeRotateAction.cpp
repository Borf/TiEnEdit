#include "NodeRotateAction.h"
#include "../TienEdit.h"

#include <VrLib/tien/Node.h>
#include <VrLib/tien/components/Transform.h>

NodeRotateAction::NodeRotateAction(vrlib::tien::Node* node)
{
	this->node = node;
	this->originalRotation = node->transform->rotation;
	this->originalPosition = node->transform->position;
}

void NodeRotateAction::perform(TienEdit * editor)
{
	node->transform->rotation = newRotation;
	node->transform->position = newPosition;
	editor->cacheSelection = true;
}

void NodeRotateAction::undo(TienEdit * editor)
{
	node->transform->rotation = originalRotation;
	node->transform->position = originalPosition;
	editor->cacheSelection = true;
}

void NodeRotateAction::updateNodePointer(vrlib::tien::Node * oldNode, vrlib::tien::Node * newNode)
{
	if (node == oldNode)
		node = newNode;
}
