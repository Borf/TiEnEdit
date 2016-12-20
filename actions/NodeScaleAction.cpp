#include "NodeScaleAction.h"
#include "../TienEdit.h"

#include <VrLib/tien/Node.h>
#include <VrLib/tien/components/Transform.h>

NodeScaleAction::NodeScaleAction(vrlib::tien::Node* node)
{
	this->node = node;
	this->originalScale = node->transform->scale;
	this->originalPosition = node->transform->position;
}

void NodeScaleAction::perform(TienEdit * editor)
{
	node->transform->scale = newScale;
	node->transform->position = newPosition;
	editor->cacheSelection = true;
}

void NodeScaleAction::undo(TienEdit * editor)
{
	node->transform->scale = originalScale;
	node->transform->position = originalPosition;
	editor->cacheSelection = true;
}
