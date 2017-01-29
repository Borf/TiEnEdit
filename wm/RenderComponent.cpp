#include "RenderComponent.h"
#include "../TienEdit.h"
#include "../actions/SelectionChangeAction.h"

#include <VrLib/json.h>
#include <VrLib/tien/Scene.h>
#include <VrLib/tien/Node.h>
#include <VrLib/tien/components/Transform.h>
#include <VrLib/tien/components/ModelRenderer.h>

RenderComponent::RenderComponent(TienEdit * editor)
{
	this->editor = editor;
}

void RenderComponent::handleDrag(DragProperties* properties)
{
	if (properties->type == DragProperties::Type::Model)
	{
		std::string fileName = properties->file;
		std::string name = fileName;
		if (name.find("/") != std::string::npos)
			name = name.substr(name.rfind("/") + 1);

		vrlib::tien::Node* n = new vrlib::tien::Node(name, &editor->tien.scene);
		n->addComponent(new vrlib::tien::components::Transform(editor->tien.scene.castRay(editor->ray, false).second));
		n->addComponent(new vrlib::tien::components::ModelRenderer(fileName));
		editor->perform(new SelectionChangeAction(editor, { n }));
	}

}
