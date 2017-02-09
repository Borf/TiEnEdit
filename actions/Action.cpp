#include "Action.h"

void Action::updateNodePointer(std::vector<vrlib::tien::Node*>& nodes, vrlib::tien::Node * oldNode, vrlib::tien::Node * newNode)
{
	for (size_t i = 0; i < nodes.size(); i++)
		if (nodes[i] == oldNode)
			nodes[i] = newNode;
}
