#include "SelectionChangeAction.h"

#include "../TienEdit.h"
#include "../wm/Tree.h"

SelectionChangeAction::SelectionChangeAction(TienEdit * editor, std::vector<vrlib::tien::Node*> newSelection)
{
	this->oldSelection = editor->objectTree->selectedItems;
	this->newSelection = newSelection;
}

void SelectionChangeAction::perform(TienEdit * editor)
{
	editor->objectTree->selectedItems = newSelection;
	editor->objectTree->update();
	editor->updateComponentsPanel();
	editor->cacheSelection = true;
}

void SelectionChangeAction::undo(TienEdit * editor)
{
	editor->objectTree->selectedItems = this->oldSelection;
	editor->objectTree->update();
	editor->updateComponentsPanel();
	editor->cacheSelection = true;
}

void SelectionChangeAction::updateNodePointer(vrlib::tien::Node * oldNode, vrlib::tien::Node * newNode)
{
	Action::updateNodePointer(oldSelection, oldNode, newNode);
	Action::updateNodePointer(newSelection, oldNode, newNode);
}
