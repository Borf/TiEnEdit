#pragma once

#include "Action.h"
#include <vector>

class GroupAction : public Action
{
public:
	std::vector<Action*> actions;


	GroupAction(const std::vector<Action*> &actions);

	virtual void perform(TienEdit * editor) override;
	virtual void undo(TienEdit * editor) override;
};