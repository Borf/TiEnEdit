#include "NodeNewAction.h"
#include <VrLib/tien/Node.h>
#include "../TienEdit.h"

NodeNewAction::NodeNewAction(vrlib::tien::Node * newNode)
{
	this->node = newNode;
}

void NodeNewAction::perform(TienEdit * editor)
{
	if (node)
	{

	}
	else
	{
		node = new vrlib::tien::Node("", &editor->tien.scene);
		node->fromJson(nodeData["node"], nodeData["meshes"]);
		if (nodeData.find("parent") != nodeData.end())
			node->setParent(editor->tien.scene.findNodeWithGuid(nodeData["parent"]));
	}
}

void NodeNewAction::undo(TienEdit * editor)
{
	nodeData["node"] = node->asJson(nodeData["meshes"]);
	if (node->parent)
		nodeData["parent"] = node->parent->guid;
	delete node;
	node = nullptr;	
	editor->objectTree->update();
}

void NodeNewAction::updateNodePointer(vrlib::tien::Node * oldNode, vrlib::tien::Node * newNode)
{
	if (node == oldNode)
		node = newNode;
}
