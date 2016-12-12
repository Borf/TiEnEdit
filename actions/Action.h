#pragma once

class TienEdit;

class Action
{
public:
	virtual void perform(TienEdit* editor) = 0;
	virtual void undo(TienEdit* editor) = 0;
};