#include "GroupAction.h"

GroupAction::GroupAction(const std::vector<Action*>& actions)
{
	this->actions = actions;
}

void GroupAction::perform(TienEdit * editor)
{
	for (auto a : actions)
		a->perform(editor);
}

void GroupAction::undo(TienEdit * editor)
{
	for (auto a : actions)
		a->undo(editor);
}

void GroupAction::updateNodePointer(vrlib::tien::Node * oldNode, vrlib::tien::Node * newNode)
{
	for (auto a : actions)
		a->updateNodePointer(oldNode, newNode);
}
