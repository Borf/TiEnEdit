#include "SelectionChangeAction.h"

#include "../TienEdit.h"
#include "../wm/Tree.h"

SelectionChangeAction::SelectionChangeAction(TienEdit * editor, std::vector<vrlib::tien::Node*> newSelection)
{
	this->oldSelection = editor->selectedNodes;
	this->newSelection = newSelection;
}

void SelectionChangeAction::perform(TienEdit * editor)
{
	editor->selectedNodes = this->newSelection;
	editor->objectTree->selectedItems.clear();
	for(auto n : newSelection)
		editor->objectTree->selectedItems.push_back(n);
	editor->objectTree->update();
	editor->objectTree->selectItem();
	editor->cacheSelection = true;
}

void SelectionChangeAction::undo(TienEdit * editor)
{
	editor->selectedNodes = this->oldSelection;
	editor->objectTree->selectedItems.clear();
	for (auto n : oldSelection)
		editor->objectTree->selectedItems.push_back(n);
	editor->objectTree->update();
	editor->objectTree->selectItem();
	editor->cacheSelection = true;
}
