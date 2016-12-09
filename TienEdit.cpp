#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include "TienEdit.h"
#include "resource.h"

#include <Windows.h>
#include <direct.h>
#include <algorithm>
#include <fstream>

#include <VrLib/gl/shader.h>
#include <vrlib/gl/FBO.h>
#include <vrlib/gl/Vertex.h>
#include <vrlib/Log.h>
#include <VrLib/Kernel.h>
#include <VrLib/ServerConnection.h>
#include <VrLib/math/aabb.h>
#include <VrLib/Model.h>
#include <vrlib/Texture.h>
#include <vrlib/gl/Cubemap.h>
#include <vrlib/TextureAtlas.h>

#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>


#include <vrlib/tien/components/Camera.h>
#include <vrlib/tien/components/Transform.h>
#include <vrlib/tien/components/Light.h>
#include <vrlib/tien/components/DynamicSkyBox.h>
#include <vrlib/tien/components/ModelRenderer.h>
#include <vrlib/tien/components/MeshRenderer.h>


#include "menu/MenuOverlay.h"
#include "menu/Menu.h"
#include "wm/SplitPanel.h"
#include "wm/RenderComponent.h"
#include "wm/Tree.h"
#include "wm/Panel.h"
#include "wm/Label.h"
#include "wm/TextField.h"
#include "wm/CheckBox.h"
#include "wm/ComboBox.h"


TienEdit::TienEdit(const std::string &fileName) : NormalApp("TiEn scene Editor")
{
}


TienEdit::~TienEdit()
{
}


class GuiEditor : public vrlib::tien::EditorBuilder
{
public:
	Panel* panel;
	float line = 0;
	std::vector<Component*> group;
	bool vertical;


	GuiEditor(Panel* panel)
	{
		this->panel = panel;
	}

	void reset()
	{
		line = 4;
	}

	virtual void addTitle(const std::string & name) override
	{
		Label* label = new Label(name, glm::ivec2(0, line));
		label->scale = 1.5f;
		label->size.y = 25;
		panel->components.push_back(label);
		line += 25;
	}


	virtual void addTextBox(const std::string & value, std::function<void(const std::string&)> onChange) override
	{
		TextField* field = new TextField(value, glm::ivec2(100, line));
		field->size.x = 200;
		field->size.y = 20;
		field->onChange = [onChange, field]()
		{
			if (onChange)
				onChange(field->value);
		};
		group.push_back(field);
	}
	virtual void addCheckbox(bool value, std::function<void(bool)> onChange) override
	{
		CheckBox* box = new CheckBox(value, glm::ivec2(100, line));
		box->onChange = [onChange, box]() { if(onChange) onChange(box->value); };
		group.push_back(box);
	}

	virtual void addButton(const std::string &value, std::function<void()> onClick) override
	{
	}

	virtual void addComboBox(const std::string &value, const std::vector<std::string> &values, std::function<void(const std::string&)> onClick) override
	{
		ComboBox* box = new ComboBox(value, glm::ivec2(100, line));
		box->values = values;
		box->size.x = 200;
		box->size.y = 20;
		box->onChange = [onClick, box]()
		{
			if(onClick)
				onClick(box->value);
		};
		group.push_back(box);
	}

	virtual void beginGroup(const std::string & name, bool verticalGroup = true) override
	{
		panel->components.push_back(new Label(name, glm::ivec2(0, line+2)));
		vertical = verticalGroup;
	}
	virtual void endGroup() override
	{
		int i = 0;
		for (auto c : group)
		{
			if (vertical)
			{
				c->position.y = line;
				line += glm::max(c->size.y, 20);
			}
			else
			{
				c->position.y = line;
				c->position.x += 200 / (group.size()) * i;
				c->size.x = 200 / group.size();
			}
			panel->components.push_back(c);
			i++;
		}



		if (!vertical)
			line += 20;
		line += 5;
		group.clear();
	}

};



void TienEdit::init()
{
	HWND hWnd = GetActiveWindow();
	HINSTANCE hInstance = GetModuleHandle(NULL);
	HICON icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL,	(LPARAM)icon);
	SendMessage(hWnd, WM_SETICON, ICON_BIG,		(LPARAM)icon);

	kernel = vrlib::Kernel::getInstance();

	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	tien.init();
	menuOverlay.init();
	menuOverlay.loadMenu("data/TiEnEdit/menu.json");

	SplitPanel* mainPanel = new SplitPanel();


	class TienNodeTree : public Tree::TreeLoader
	{
		TienEdit* edit;
	public:
		TienNodeTree(TienEdit* edit)
		{
			this->edit = edit;
		}
		virtual std::string getName(void* data)
		{
			if (data == nullptr)
				return "root";
			auto n = static_cast<vrlib::tien::Node*>(data);
			return n->name;
		}
		virtual int getChildCount(void* data)
		{
			if (data == nullptr)
				return edit->tien.scene.getChildren().size();
			auto n = static_cast<vrlib::tien::Node*>(data);
			return n->getChildren().size();
		}
		virtual void* getChild(void* data, int index)
		{
			if (data == nullptr)
				return edit->tien.scene.getChild(index);
			else
			{
				auto n = static_cast<vrlib::tien::Node*>(data);
				return n->getChild(index);
			}
		}
		virtual int getIcon(void* data)
		{
			auto n = static_cast<vrlib::tien::Node*>(data);
			if (n->getComponent<vrlib::tien::components::ModelRenderer>())
				return 2;
			if (n->getComponent<vrlib::tien::components::MeshRenderer>())
				return 2;
			if (n->getComponent<vrlib::tien::components::Camera>())
				return 4;
			if (n->getComponent<vrlib::tien::components::Light>())
				return 5;

			return 3;
		}
	};

	Tree* objectTree = new Tree();
	objectTree->loader = new TienNodeTree(this);
	mainPanel->addPanel(objectTree);
	
	mainPanel->addPanel(renderPanel = new RenderComponent());
	Panel* propertiesPanel = new Panel();
	mainPanel->addPanel(propertiesPanel);


	GuiEditor* editorBuilder = new GuiEditor(propertiesPanel);

	objectTree->rightClickItem = [this]()
	{
		vrlib::json::Value popupMenu = vrlib::json::readJson(std::ifstream("data/TiEnEdit/nodemenu.json"));

		menuOverlay.popupMenus.push_back(std::pair<glm::vec2, Menu*>(mouseState.pos, new Menu(popupMenu)));
	};
	objectTree->selectItem = [this, mainPanel, propertiesPanel, objectTree, editorBuilder]()
	{
		vrlib::tien::Node* node = static_cast<vrlib::tien::Node*>(objectTree->selectedItem);

		for (auto c : propertiesPanel->components)
			delete c;
		propertiesPanel->components.clear();
		editorBuilder->reset();

		editorBuilder->addTitle("Node:");
		
		editorBuilder->beginGroup("Name");
		editorBuilder->addTextBox(node->name, [node, objectTree](const std::string &newValue) { node->name = newValue; objectTree->update(); });
		editorBuilder->endGroup();

		editorBuilder->beginGroup("GUID");
		editorBuilder->addTextBox(node->guid, [node](const std::string &newValue) { node->guid = newValue; });
		editorBuilder->endGroup();

		editorBuilder->beginGroup("Parent");
		editorBuilder->addTextBox(node->parent ? node->parent->name : "-", [node](const std::string &newValue) {  });
		editorBuilder->endGroup();

		std::vector<vrlib::tien::Component*> components = node->getComponents();
		for (auto c : components)
			c->buildEditor(editorBuilder);
		propertiesPanel->onReposition(mainPanel);

		editorBuilder->addTitle("");
		editorBuilder->beginGroup("Add Component");
		editorBuilder->addButton("Add", [node]() {});
		editorBuilder->endGroup();

	};

	panel = mainPanel;
	panel->position = glm::ivec2(0, 25+36);
	panel->size = glm::ivec2(kernel->getWindowWidth(), kernel->getWindowHeight());
	mainPanel->sizes[0] = 300;
	mainPanel->sizes[2] = 300;
	mainPanel->sizes[1] = mainPanel->size.x - 600;
	panel->onReposition(nullptr);

	vrlib::tien::Node* sunlight;
	{
		vrlib::tien::Node* n = new vrlib::tien::Node("Sunlight", &tien.scene);
		n->addComponent(new vrlib::tien::components::Transform(glm::vec3(0, 1, 1)));
		vrlib::tien::components::Light* light = new vrlib::tien::components::Light();
		light->color = glm::vec4(1, 1, 0.8627f, 1);
		light->intensity = 20.0f;
		light->type = vrlib::tien::components::Light::Type::directional;
		light->shadow = vrlib::tien::components::Light::Shadow::shadowmap;
		n->addComponent(light);
		sunlight = n;
	}

	{
		vrlib::tien::Node* n = new vrlib::tien::Node("Camera", &tien.scene);
		n->addComponent(new vrlib::tien::components::Transform(glm::vec3(0,1.5f, -1.5f)));
		n->addComponent(new vrlib::tien::components::Camera());
		n->addComponent(new vrlib::tien::components::DynamicSkyBox());
		n->getComponent<vrlib::tien::components::DynamicSkyBox>()->light = sunlight;
		tien.scene.cameraNode = n;
	}

	/*vrlib::tien::Node* parent;
	{
		vrlib::tien::Node* n = new vrlib::tien::Node("GroundPlane", &tien.scene);
		n->addComponent(new vrlib::tien::components::Transform(glm::vec3(0, -0.01f, 0)));

		vrlib::tien::components::MeshRenderer::Mesh* mesh = new vrlib::tien::components::MeshRenderer::Mesh();
		mesh->material.texture = vrlib::Texture::loadCached("data/NetworkEngine/textures/grid.png");
		mesh->indices = { 2, 1, 0, 2, 0, 3 };
		vrlib::gl::VertexP3N2B2T2T2 v;
		vrlib::gl::setN3(v, glm::vec3(0, 1, 0));
		vrlib::gl::setTan3(v, glm::vec3(1, 0, 0));
		vrlib::gl::setBiTan3(v, glm::vec3(0, 0, 1));

		vrlib::gl::setP3(v, glm::vec3(-100, 0, -100));		vrlib::gl::setT2(v, glm::vec2(-25, -25));		mesh->vertices.push_back(v);
		vrlib::gl::setP3(v, glm::vec3(100, 0, -100));		vrlib::gl::setT2(v, glm::vec2(25, -25));		mesh->vertices.push_back(v);
		vrlib::gl::setP3(v, glm::vec3(100, 0, 100));		vrlib::gl::setT2(v, glm::vec2(25, 25));		mesh->vertices.push_back(v);
		vrlib::gl::setP3(v, glm::vec3(-100, 0, 100));		vrlib::gl::setT2(v, glm::vec2(-25, 25));		mesh->vertices.push_back(v);

		n->addComponent(new vrlib::tien::components::MeshRenderer(mesh));
		parent = n;
	}

	{
		vrlib::tien::Node* n = new vrlib::tien::Node("OtherPlane", parent);
		n->addComponent(new vrlib::tien::components::Transform(glm::vec3(0, 1, -1)));

		vrlib::tien::components::MeshRenderer::Mesh* mesh = new vrlib::tien::components::MeshRenderer::Mesh();
		mesh->material.texture = vrlib::Texture::loadCached("data/NetworkEngine/textures/grid.png");
		mesh->indices = { 2, 1, 0, 2, 0, 3 };
		vrlib::gl::VertexP3N2B2T2T2 v;
		vrlib::gl::setN3(v, glm::vec3(0, 1, 0));
		vrlib::gl::setTan3(v, glm::vec3(1, 0, 0));
		vrlib::gl::setBiTan3(v, glm::vec3(0, 0, 1));

		vrlib::gl::setP3(v, glm::vec3(-1, 0, -1));		vrlib::gl::setT2(v, glm::vec2(-1, -1));		mesh->vertices.push_back(v);
		vrlib::gl::setP3(v, glm::vec3(1, 0, -1));		vrlib::gl::setT2(v, glm::vec2(1, -1));		mesh->vertices.push_back(v);
		vrlib::gl::setP3(v, glm::vec3(1, 0, 1));		vrlib::gl::setT2(v, glm::vec2(1, 1));		mesh->vertices.push_back(v);
		vrlib::gl::setP3(v, glm::vec3(-1, 0, 1));		vrlib::gl::setT2(v, glm::vec2(-1, 1));		mesh->vertices.push_back(v);

		n->addComponent(new vrlib::tien::components::MeshRenderer(mesh));
	}*/
	{
		vrlib::tien::Node* n = new vrlib::tien::Node("Model", &tien.scene);
		n->addComponent(new vrlib::tien::components::Transform(glm::vec3(0, 0, -8), glm::quat(), glm::vec3(1.0f, 1.0f, 1.0f)));
		n->addComponent(new vrlib::tien::components::ModelRenderer("data/Models/vangogh/Enter a title.obj"));
		n->getComponent<vrlib::tien::components::ModelRenderer>()->castShadow = false;
		//n->addComponent(new vrlib::tien::components::ModelRenderer("data/Models/swamp/map_1.obj"));
		
	}

	focussedComponent = renderPanel;

	objectTree->update();


	tien.start();

}

void TienEdit::preFrame(double frameTime, double totalTime)
{
	menuOverlay.setWindowSize(kernel->getWindowSize());
	menuOverlay.hover();

	panel->size = glm::ivec2(kernel->getWindowWidth(), kernel->getWindowHeight() - 25 - 36);
	panel->onReposition(nullptr);


	if (focussedComponent == renderPanel || mouseState.middle)
	{
		glm::vec3 cameraMovement(0, 0, 0);
		if (KeyboardDeviceDriver::isPressed(KeyboardDeviceDriver::KEY_W))		cameraMovement.z = -1;
		if (KeyboardDeviceDriver::isPressed(KeyboardDeviceDriver::KEY_S))		cameraMovement.z = 1;
		if (KeyboardDeviceDriver::isPressed(KeyboardDeviceDriver::KEY_A))		cameraMovement.x = -1;
		if (KeyboardDeviceDriver::isPressed(KeyboardDeviceDriver::KEY_D))		cameraMovement.x = 1;
		if (KeyboardDeviceDriver::isPressed(KeyboardDeviceDriver::KEY_Q))		cameraMovement.y = 1;
		if (KeyboardDeviceDriver::isPressed(KeyboardDeviceDriver::KEY_Z))		cameraMovement.y = -1;

		cameraMovement *= frameTime / 100.0f;
		cameraPos += cameraMovement * cameraRot;
	}


	if (mouseState.middle)
	{
		cameraRot = cameraRot * glm::quat(glm::vec3(0, .01f * (mouseState.pos.x - lastMouseState.pos.x), 0));
		cameraRot = glm::quat(glm::vec3(.01f * (mouseState.pos.y - lastMouseState.pos.y), 0, 0)) * cameraRot;
	}



	tien.update((float)(frameTime / 1000.0f));


	lastMouseState = mouseState;
}


void TienEdit::draw()
{
	float aspect = (float)kernel->getWindowWidth() / kernel->getWindowHeight();


	glViewport(0, 0, kernel->getWindowWidth(), kernel->getWindowHeight());
	menuOverlay.drawInit();
	menuOverlay.drawRootMenu();
	panel->draw(&menuOverlay);
	menuOverlay.drawPopups();

	
	glm::mat4 cameraMat = glm::translate(glm::toMat4(cameraRot), -cameraPos);


	glm::mat4 projectionMatrix = glm::perspective(70.0f, renderPanel->size.x / (float)renderPanel->size.y, 0.01f, 500.0f);
	glm::mat4 modelViewMatrix = cameraMat;


	//glViewport(renderPanel->position.x, renderPanel->position.y, renderPanel->size.x, renderPanel->size.y);
	glViewport(renderPanel->position.x, kernel->getWindowHeight() - renderPanel->position.y - renderPanel->size.y, renderPanel->size.x, renderPanel->size.y);
	tien.render(projectionMatrix, modelViewMatrix);
}

void TienEdit::mouseMove(int x, int y)
{
	mouseState.pos.x = x;
	mouseState.pos.y = y;

	//mousePos.x = (float)x;
	//mousePos.y = (float)y;
	menuOverlay.mousePos = glm::vec2(x, y);
}

void TienEdit::mouseScroll(int offset)
{
	//if (overlay)
	//	overlay->scroll(offset);
}


void TienEdit::mouseDown(MouseButton button)
{
	mouseState.buttons[(int)button] = true;
}

void TienEdit::keyDown(int button)
{
	NormalApp::keyDown(button);
}

void TienEdit::keyUp(int button)
{
	NormalApp::keyUp(button);
	if (focussedComponent)
		focussedComponent->keyUp(button);
}

void TienEdit::keyChar(char character)
{
	NormalApp::keyChar(character);
	if (focussedComponent)
		focussedComponent->keyChar(character);
}


void TienEdit::mouseUp(MouseButton button)
{
	mouseState.buttons[(int)button] = false;
	//TODO if click
	if(button != vrlib::MouseButtonDeviceDriver::MouseButton::Middle)
	{
		if (menuOverlay.click(button == vrlib::MouseButtonDeviceDriver::MouseButton::Left))
			return;

		Component* clickedComponent = panel->getComponentAt(mouseState.pos);

		if (focussedComponent != clickedComponent)
		{
			if (focussedComponent)
			{
				focussedComponent->focussed = false;
				focussedComponent->unfocus();
			}
			focussedComponent = clickedComponent;
			if (focussedComponent)
			{
				focussedComponent->focussed = true;
				focussedComponent->focus();
			}
		}


		if (panel->click(button == vrlib::MouseButtonDeviceDriver::MouseButton::Left, mouseState.pos))
			return;




	}


}

