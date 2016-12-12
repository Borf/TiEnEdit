#include "SelectionChangeAction.h"

#include "../TienEdit.h"

SelectionChangeAction::SelectionChangeAction(TienEdit * editor, std::vector<vrlib::tien::Node*> newSelection)
{
	this->oldSelection = editor->selectedNodes;
	this->newSelection = newSelection;
}

void SelectionChangeAction::perform(TienEdit * editor)
{
	editor->selectedNodes = this->newSelection;
	editor->cacheSelection = true;
}

void SelectionChangeAction::undo(TienEdit * editor)
{
	editor->selectedNodes = this->oldSelection;
	editor->cacheSelection = true;
}
