#include "NodeScaleAction.h"
#include "../TienEdit.h"

#include <VrLib/tien/Node.h>
#include <VrLib/tien/components/Transform.h>

NodeScaleAction::NodeScaleAction(vrlib::tien::Node* node)
{
	this->node = node;
	this->originalScale = node->transform->scale;
}

void NodeScaleAction::perform(TienEdit * editor)
{
	node->transform->scale = newScale;
	editor->cacheSelection = true;
}

void NodeScaleAction::undo(TienEdit * editor)
{
	node->transform->scale = originalScale;
	editor->cacheSelection = true;
}
