#pragma once

#include "Action.h"
#include <vector>
#include <VrLib/json.hpp>

namespace vrlib { namespace tien { class Node;  } }

class NodeDeleteAction : public Action
{
	std::vector<vrlib::tien::Node*> selectedNodes;
	json data;
public:
	NodeDeleteAction(const std::vector<vrlib::tien::Node*> &selectedNodes);

	virtual void perform(TienEdit * editor) override;

	virtual void undo(TienEdit * editor) override;

	virtual void updateNodePointer(vrlib::tien::Node * oldNode, vrlib::tien::Node * newNode) override;

};