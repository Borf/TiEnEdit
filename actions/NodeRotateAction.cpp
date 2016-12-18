#include "NodeRotateAction.h"
#include "../TienEdit.h"

#include <VrLib/tien/Node.h>
#include <VrLib/tien/components/Transform.h>

NodeRotateAction::NodeRotateAction(vrlib::tien::Node* node)
{
	this->node = node;
	this->originalRotation = node->transform->rotation;
}

void NodeRotateAction::perform(TienEdit * editor)
{
	node->transform->rotation = newRotation;
	editor->cacheSelection = true;
}

void NodeRotateAction::undo(TienEdit * editor)
{
	node->transform->rotation = originalRotation;
	editor->cacheSelection = true;
}
