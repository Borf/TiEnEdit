#pragma once


#include "Action.h"

#include <vector>

namespace vrlib { namespace tien { class Node;  } }

class SelectionChangeAction : public Action
{
	std::vector<vrlib::tien::Node*> oldSelection;
	std::vector<vrlib::tien::Node*> newSelection;
public:
	SelectionChangeAction(TienEdit* editor, std::vector<vrlib::tien::Node*> newSelection);

	virtual void perform(TienEdit* editor) override;
	virtual void undo(TienEdit* editor) override;

	virtual void updateNodePointer(vrlib::tien::Node * oldNode, vrlib::tien::Node * newNode) override;

};