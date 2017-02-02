#include "RenderComponent.h"
#include "../TienEdit.h"
#include "../actions/SelectionChangeAction.h"

#include <VrLib/json.hpp>
#include <VrLib/Texture.h>
#include <VrLib/tien/Scene.h>
#include <VrLib/tien/Node.h>
#include <VrLib/tien/components/Transform.h>
#include <VrLib/tien/components/ModelRenderer.h>
#include <VrLib/tien/components/MeshRenderer.h>
#include <VrLib/math/Plane.h>

RenderComponent::RenderComponent(TienEdit * editor)
{
	this->editor = editor;
}

void RenderComponent::handleDrag(DragProperties* properties)
{
	if (!properties)
		return;


	auto rayCast = editor->tien.scene.castRay(editor->ray, false);
	auto mousePos = rayCast.second;
	if (!rayCast.first)
	{
		vrlib::math::Plane plane(glm::vec3(0, 1, 0), 0);
		mousePos = plane.getCollisionPoint(editor->ray);
	}

	if (properties->type == DragProperties::Type::Model)
	{
		std::string fileName = properties->file;
		std::string name = fileName;
		if (name.find("/") != std::string::npos)
			name = name.substr(name.rfind("/") + 1);

		vrlib::tien::Node* n = new vrlib::tien::Node(name, &editor->tien.scene);
		n->addComponent(new vrlib::tien::components::Transform(mousePos));
		n->addComponent(new vrlib::tien::components::ModelRenderer(fileName));
		editor->perform(new SelectionChangeAction(editor, { n }));
	}
	if (properties->type == DragProperties::Type::Texture)
	{
		std::string fileName = properties->file;
		std::string name = fileName;
		if (name.find("/") != std::string::npos)
			name = name.substr(name.rfind("/") + 1);

		vrlib::tien::Node* n = new vrlib::tien::Node(name, &editor->tien.scene);
		n->addComponent(new vrlib::tien::components::Transform(mousePos));


		auto mesh = new vrlib::tien::components::MeshRenderer::Mesh();

		mesh->vertices.push_back(vrlib::gl::VertexP3N2B2T2T2(glm::vec3(-1, 0, -1), glm::vec3(0, 1, 0), glm::vec2(0, 1), glm::vec3(1, 0, 0)));
		mesh->vertices.push_back(vrlib::gl::VertexP3N2B2T2T2(glm::vec3( 1, 0, -1), glm::vec3(0, 1, 0), glm::vec2(1, 1), glm::vec3(1, 0, 0)));
		mesh->vertices.push_back(vrlib::gl::VertexP3N2B2T2T2(glm::vec3( 1, 0,  1), glm::vec3(0, 1, 0), glm::vec2(1, 0), glm::vec3(1, 0, 0)));
		mesh->vertices.push_back(vrlib::gl::VertexP3N2B2T2T2(glm::vec3(-1, 0,  1), glm::vec3(0, 1, 0), glm::vec2(0, 0), glm::vec3(1, 0, 0)));

		mesh->indices = { 2, 1, 0, 3, 2, 0 };

		mesh->material.texture = vrlib::Texture::loadCached(fileName);

		n->addComponent(new vrlib::tien::components::MeshRenderer(mesh));
		editor->perform(new SelectionChangeAction(editor, { n }));
	}
}
