#include "NodeDeleteAction.h"
#include "../TienEdit.h"
#include <VrLib/util.h>
#include <VrLib/Log.h>
using vrlib::logger;

NodeDeleteAction::NodeDeleteAction(const std::vector<vrlib::tien::Node*>& selectedNodes)
{
	this->selectedNodes = selectedNodes;

}

void NodeDeleteAction::perform(TienEdit * editor)
{
	for (auto c : selectedNodes)
	{
		if (editor->checkNodePointer(c))
		{
			logger << "Node is still in use" << Log::newline;
			return;
		}
	}


	for (auto c : selectedNodes)
	{
		nlohmann::json node = c->asJson(data["meshes"]);
		if(c->parent)
			node["parent"] = c->parent->guid;
		data["nodes"].push_back(node);
		delete c;
	}
	editor->objectTree->selectedItems.clear();
	editor->objectTree->update();
	editor->updateComponentsPanel();
	editor->cacheSelection = true;

}

void NodeDeleteAction::undo(TienEdit * editor)
{
	for (const nlohmann::json &n : data["nodes"])
	{
		vrlib::tien::Node* newNode = new vrlib::tien::Node("", &editor->tien.scene);
		newNode->fromJson(n, data, [](const nlohmann::json &, std::string) { return nullptr; });
		if (n.find("parent") != n.end())
			newNode->setParent(editor->tien.scene.findNodeWithGuid(n["parent"]));
		editor->objectTree->selectedItems.push_back(newNode);
	}
	editor->objectTree->update();
	editor->updateComponentsPanel();
	editor->cacheSelection = true;

	for (size_t i = 0; i < selectedNodes.size(); i++)
	{ //TODO: assert that these node pointers match
		editor->updateNodePointer(this->selectedNodes[i], editor->objectTree->selectedItems[i]);
	}
}

void NodeDeleteAction::updateNodePointer(vrlib::tien::Node * oldNode, vrlib::tien::Node * newNode)
{
	Action::updateNodePointer(selectedNodes, oldNode, newNode);
}
