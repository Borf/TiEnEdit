#pragma once

#include "Action.h"
#include <VrLib/json.hpp>

class NodeNewAction : public Action
{
	json nodeData;
	vrlib::tien::Node* node;
public:
	NodeNewAction(vrlib::tien::Node* newNode);

	// Inherited via Action
	virtual void perform(TienEdit * editor) override;
	virtual void undo(TienEdit * editor) override;
	virtual void updateNodePointer(vrlib::tien::Node * oldNode, vrlib::tien::Node * newNode) override;
};