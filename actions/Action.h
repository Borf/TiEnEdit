#pragma once

#include <vector>

namespace vrlib { namespace tien { class Node; } }
class TienEdit;


class Action
{
public:
	virtual void perform(TienEdit* editor) = 0;
	virtual void undo(TienEdit* editor) = 0;
	virtual void updateNodePointer(vrlib::tien::Node* oldNode, vrlib::tien::Node* newNode) = 0;
	virtual void updateNodePointer(std::vector<vrlib::tien::Node*> &nodes, vrlib::tien::Node* oldNode, vrlib::tien::Node* newNode);
};