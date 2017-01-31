#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include "TienEdit.h"
#include "resource.h"

#include <Windows.h>
#include <direct.h>
#include <algorithm>
#include <fstream>
#include <sstream>

#include <VrLib/gl/shader.h>
#include <vrlib/gl/FBO.h>
#include <vrlib/gl/Vertex.h>
#include <vrlib/Log.h>
#include <VrLib/Kernel.h>
#include <VrLib/math/aabb.h>
#include <VrLib/math/Plane.h>
#include <VrLib/util.h>
#include <VrLib/Texture.h>

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
#include <vrlib/tien/components/StaticSkyBox.h>
#include <vrlib/tien/components/DynamicSkyBox.h>
#include <vrlib/tien/components/AnimatedModelRenderer.h>
#include <vrlib/tien/components/ModelRenderer.h>
#include <vrlib/tien/components/MeshRenderer.h>
#include <vrlib/tien/components/TerrainRenderer.h>
#include <VrLib/tien/components/RigidBody.h>
#include <VrLib/tien/components/BoxCollider.h>
#include <VrLib/tien/components/MeshCollider.h>
#include <VrLib/tien/components/SphereCollider.h>
#include <VrLib/tien/components/TerrainCollider.h>

#include "actions/Action.h"
#include "menu/MenuOverlay.h"
#include "menu/Menu.h"
#include "wm/SplitPanel.h"
#include "wm/RenderComponent.h"
#include "wm/Tree.h"
#include "wm/Panel.h"
#include "wm/Label.h"
#include "wm/Image.h"
#include "wm/Button.h"
#include "wm/ScrollPanel.h"

#include "actions/SelectionChangeAction.h"
#include "actions/GroupAction.h"
#include "actions/NodeMoveAction.h"
#include "actions/NodeScaleAction.h"
#include "actions/NodeRotateAction.h"


#include "EditorBuilderGui.h"
#include "BrowsePanel.h"


#define SCENEWIDTH 300
#define PROPERTYWIDTH 300
#define BROWSERHEIGHT 300



class Rotator : public vrlib::tien::Component
{
	virtual void update(float elapsedTime, vrlib::tien::Scene& scene)
	{
		node->transform->rotate(glm::vec3(0, elapsedTime, 0));
	}
	virtual vrlib::json::Value toJson(vrlib::json::Value & meshes) const override
	{
		vrlib::json::Value ret;
		ret["type"] = "rotate";
		return ret;
	}
};


class StubComponent : public vrlib::tien::Component
{
public:
	vrlib::json::Value stub;
	virtual vrlib::json::Value toJson(vrlib::json::Value & meshes) const override
	{
		vrlib::json::Value ret;
		ret["type"] = stub["type"];
		return ret;
	}
};



typedef std::pair<std::function<vrlib::tien::Component*(vrlib::tien::Node*)>, std::function<vrlib::tien::Component*(const vrlib::json::Value&, const vrlib::json::Value&)>> ComponentPair;
std::map<std::string, ComponentPair> componentFactory =
{
	{ "Mesh Renderer", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::MeshRenderer(); }, 
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::MeshRenderer(json, totalJson); }) },
	{ "Model Renderer", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::ModelRenderer(""); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::ModelRenderer(json); }) },
	{ "Animated Model Renderer", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::AnimatedModelRenderer(""); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::AnimatedModelRenderer(json); }) },
	{ "Terrain Renderer", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::TerrainRenderer(nullptr); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::TerrainRenderer(nullptr); }) },
	{ "Light", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::Light(); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::Light(json); }) },
	{ "Static Skybox", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::StaticSkyBox(); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::StaticSkyBox(); }) },
	{ "Dynamic Skybox", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::DynamicSkyBox(); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::DynamicSkyBox(json); }) },
	{ "Camera", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::Camera(); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::Camera(json); }) },
	{ "RigidBody", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::RigidBody(0.0f, vrlib::tien::components::RigidBody::Type::Static); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::RigidBody(json); }) },
	{ "BoxCollider", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::BoxCollider(n); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::BoxCollider(json); }) },
	{ "MeshCollider", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::MeshCollider(n, true); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { throw "ugh";  return new vrlib::tien::components::MeshCollider(nullptr, true); }) },
	{ "SphereCollider", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::SphereCollider(); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::SphereCollider(); }) },
	{ "TerrainCollider", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::TerrainCollider(n); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::TerrainCollider(); }) },
	{ "rotator", ComponentPair(
		[](vrlib::tien::Node* n) { return new Rotator(); },
		[](const vrlib::json::Value &json, const vrlib::json::Value &totalJson) { return new vrlib::tien::components::ModelRenderer(json); }) },


};




TienEdit::TienEdit(const std::string &fileName) : NormalApp("TiEn scene Editor")
{
	this->fileName = fileName;
}


TienEdit::~TienEdit()
{
}






void TienEdit::init()
{
	//set icon
	HWND hWnd = GetActiveWindow();
	HINSTANCE hInstance = GetModuleHandle(NULL);
	HICON icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	SendMessage(hWnd, WM_SETICON, ICON_SMALL,	(LPARAM)icon);
	SendMessage(hWnd, WM_SETICON, ICON_BIG,		(LPARAM)icon);

	kernel = vrlib::Kernel::getInstance();
	//initialize Open dialog
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	//load stubs for different applications
	vrlib::json::Value stubs = vrlib::json::readJson(std::ifstream("data/TiEnEdit/stubs.json"));
	for (const vrlib::json::Value &stub : stubs)
	{
		componentFactory[stub["name"]] = ComponentPair([stub](vrlib::tien::Node* n)
		{
			StubComponent* c = new StubComponent();
			c->stub = stub;
			return c;
		},
		[stub](const vrlib::json::Value &json, const vrlib::json::Value &totalJson)
		{
			StubComponent* c = new StubComponent();
			c->stub = stub;
			return c;
		});
	}



	//scaling ruler model
	ruler = vrlib::Model::getModel<vrlib::gl::VertexP3T2>("data/TiEnEdit/models/ruler1/ruler1.fbx");

	shader = new vrlib::gl::Shader<EditorShaderUniforms>("data/TiEnEdit/shaders/editor.vert", "data/TiEnEdit/shaders/editor.frag");
	shader->bindAttributeLocation("a_position", 0);
	shader->bindAttributeLocation("a_texcoord", 1);
	shader->link();
	shader->bindFragLocation("fragColor", 0);
	shader->registerUniform(EditorShaderUniforms::projectionMatrix, "projectionMatrix");
	shader->registerUniform(EditorShaderUniforms::modelViewMatrix, "modelViewMatrix");
	shader->registerUniform(EditorShaderUniforms::s_texture, "s_texture");
	shader->registerUniform(EditorShaderUniforms::textureFactor, "textureFactor");
	shader->registerUniform(EditorShaderUniforms::color, "color");
	shader->use();
	shader->setUniform(EditorShaderUniforms::s_texture, 0);

	tien.init();

	//initialize menus
	menuOverlay.init();
	menuOverlay.loadMenu("data/TiEnEdit/menu.json");
	menuOverlay.rootMenu->setAction("file/new", [this]() {tien.scene.reset(); {		vrlib::tien::Node* n = new vrlib::tien::Node("Camera", &tien.scene);		n->addComponent(new vrlib::tien::components::Transform(glm::vec3(0, 0, 0)));		n->addComponent(new vrlib::tien::components::Camera());		n->addComponent(new vrlib::tien::components::DynamicSkyBox());		tien.scene.cameraNode = n;	} });
	menuOverlay.rootMenu->setAction("file/open", std::bind(&TienEdit::load, this));
	menuOverlay.rootMenu->setAction("file/save", std::bind(&TienEdit::save, this));

	menuOverlay.rootMenu->setAction("edit/undo", std::bind(&TienEdit::undo, this));
	menuOverlay.rootMenu->setAction("edit/redo", std::bind(&TienEdit::redo, this));
	menuOverlay.rootMenu->setAction("edit/sort", std::bind(&TienEdit::sortScene, this));

	menuOverlay.rootMenu->setAction("object/copy", std::bind(&TienEdit::copy, this));
	menuOverlay.rootMenu->setAction("object/paste", std::bind(&TienEdit::paste, this));
	menuOverlay.rootMenu->setAction("object/delete", std::bind(&TienEdit::deleteSelection, this));

	//build up UI
	mainPanel = new SplitPanel(SplitPanel::Alignment::HORIZONTAL);
	class TienNodeTree : public Tree<vrlib::tien::Node*>::TreeLoader
	{
		TienEdit* edit;
	public:
		TienNodeTree(TienEdit* edit)
		{
			this->edit = edit;
		}
		virtual std::string getName(vrlib::tien::Node* n)
		{
			if (n == nullptr)
				return "root";
			return n->name;
		}
		virtual int getChildCount(vrlib::tien::Node* n)
		{
			if (n == nullptr)
				return edit->tien.scene.getChildren().size();
			return n->getChildren().size();
		}
		virtual vrlib::tien::Node* getChild(vrlib::tien::Node* n, int index)
		{
			if (n == nullptr)
				return edit->tien.scene.getChild(index);
			else
				return n->getChild(index);
		}
		virtual int getIcon(vrlib::tien::Node* n)
		{
			if (n->getComponent<vrlib::tien::components::ModelRenderer>() || n->getComponent<vrlib::tien::components::AnimatedModelRenderer>())
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
	objectTree = new Tree<vrlib::tien::Node*>();
	objectTree->loader = new TienNodeTree(this);
	mainPanel->addPanel(objectTree);
	
	SplitPanel* subPanel = new SplitPanel(SplitPanel::Alignment::VERTICAL);
	subPanel->addPanel(renderPanel = new RenderComponent(this));
	subPanel->addPanel(new ScrollPanel(browsePanel = new BrowsePanel(this)));
	subPanel->sizes[1] = BROWSERHEIGHT;
	subPanel->sizes[0] = mainPanel->size.y - BROWSERHEIGHT;

	mainPanel->addPanel(subPanel);
	propertiesPanel = new Panel();
	propertiesPanel->size.x = PROPERTYWIDTH;
	propertiesPanel->size.y = 100000;
	mainPanel->addPanel(new ScrollPanel( propertiesPanel ));


	editorBuilder = new GuiEditor(this, propertiesPanel);

	objectTree->rightClickItem = [this]()
	{
		if (!objectTree->selectedItems.empty())
		{
			Menu* menu = new Menu(vrlib::json::readJson(std::ifstream("data/TiEnEdit/nodemenu.json")));
			menuOverlay.popupMenus.push_back(std::pair<glm::vec2, Menu*>(mouseState.pos, menu));
			menu->setAction("delete", std::bind(&TienEdit::deleteSelection, this));
			menu->setAction("focus with camera", std::bind(&TienEdit::focusSelectedObject, this));
			menu->setAction("add debug", [this]() { for (auto i : objectTree->selectedItems) { i->addDebugChildSphere(); } });
		}
		else
		{
			vrlib::json::Value popupMenu = vrlib::json::readJson(std::ifstream("data/TiEnEdit/newnodemenu.json"));
			Menu* menu = new Menu(popupMenu);
			menuOverlay.popupMenus.push_back(std::pair<glm::vec2, Menu*>(mouseState.pos, menu));
			menu->setAction("new node", [this]()
			{
				vrlib::tien::Node* n = new vrlib::tien::Node("new node", &tien.scene);
				n->addComponent(new vrlib::tien::components::Transform());
				perform(new SelectionChangeAction(this, { n }));
			});
		}
	};
	objectTree->selectItem = [this]()
	{
		perform(new SelectionChangeAction(this, objectTree->selectedItems));
	};
	objectTree->doubleClickItem = std::bind(&TienEdit::focusSelectedObject, this);
	objectTree->dragItem = [this](vrlib::tien::Node* from, vrlib::tien::Node* to)
	{//TODO: undo
		if (!to)
			to = &tien.scene;
		from->setParent(to); //TODO: keep transforms
		objectTree->update();
	};



	mainPanel->position = glm::ivec2(0, 25+36);
	mainPanel->size = glm::ivec2(kernel->getWindowWidth(), kernel->getWindowHeight());
	mainPanel->sizes[0] = SCENEWIDTH;
	mainPanel->sizes[2] = PROPERTYWIDTH;
	mainPanel->sizes[1] = mainPanel->size.x - SCENEWIDTH-PROPERTYWIDTH;
	dynamic_cast<SplitPanel*>(mainPanel->components[1])->sizes[0] = mainPanel->size.y - BROWSERHEIGHT;
	mainPanel->onReposition(nullptr);

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
		n->addComponent(new vrlib::tien::components::Transform(glm::vec3(0,0, 0)));
		n->addComponent(new vrlib::tien::components::Camera());
		n->addComponent(new vrlib::tien::components::DynamicSkyBox());
		n->getComponent<vrlib::tien::components::DynamicSkyBox>()->light = sunlight;
	}

	/*{
		vrlib::tien::Node* n = new vrlib::tien::Node("Model", &tien.scene);
		n->addComponent(new vrlib::tien::components::Transform(glm::vec3(0, 0, 0), glm::quat(), glm::vec3(1,1,1)));
		n->addComponent(new vrlib::tien::components::ModelRenderer("data/Models/vangogh/Enter a title.obj"));

		//n->addComponent(new vrlib::tien::components::ModelRenderer("data/tientest/models/WoodenBox02.obj"));
//		n->addComponent(new vrlib::tien::components::Transform(glm::vec3(0, 0, 0), glm::quat(), glm::vec3(0.01f, 0.01f, 0.01f)));
//		n->addComponent(new vrlib::tien::components::ModelRenderer("data/tientest/models/crytek-sponza/sponza.obj"));
		n->getComponent<vrlib::tien::components::ModelRenderer>()->castShadow = false;
	}*/
	/*vrlib::json::Value v = vrlib::json::readJson(std::ifstream("data/virtueelpd/scenes/Real PD1 v1-6.json"));
	for (const auto &o : v["objects"])
	{
		if (o["model"].asString().find(".fbx") != std::string::npos)
			o["model"] = o["model"].asString().substr(0, o["model"].asString().size() - 4);


		std::string fileName = "data/virtueelpd/models/" + o["category"].asString() + "/" + o["model"].asString() + "/" + o["model"].asString() + ".fbx";
		std::ifstream file(fileName);
		if (file.is_open())
		{
			file.close();
			vrlib::tien::Node* n = new vrlib::tien::Node(o["model"], &tien.scene);
			n->addComponent(new vrlib::tien::components::Transform(
				glm::vec3(o["x"], o["y"], -o["z"].asFloat() + 15),
				glm::quat(o["rotationquat"]["w"], o["rotationquat"]["x"], o["rotationquat"]["y"], o["rotationquat"]["z"]),
				glm::vec3(o["scale"], o["scale"], o["scale"]) * 40.0f));
			n->addComponent(new vrlib::tien::components::ModelRenderer(fileName));

			if (o["model"].asString().find("realPD1") != std::string::npos)
				n->getComponent<vrlib::tien::components::ModelRenderer>()->castShadow = false;
		}
		else
			vrlib::logger << "Could not find file " << fileName << vrlib::Log::newline;
	}*/


	focussedComponent = renderPanel;

	objectTree->update();


	cameraPos = glm::vec3(0, 1.8f, 8.0f);
	load();
	tien.start();
	tien.pause();

}

void TienEdit::preFrame(double frameTime, double totalTime)
{
	menuOverlay.setWindowSize(kernel->getWindowSize());
	menuOverlay.hover();

	mainPanel->size = glm::ivec2(kernel->getWindowWidth(), kernel->getWindowHeight() - 25 - 36);
	mainPanel->sizes[1] = mainPanel->size.x - 600;
	mainPanel->onReposition(nullptr); //TODO: don't do this every frame ._.;


	if (mouseState.middle)
	{
		glm::vec3 cameraMovement(0, 0, 0);
		if (KeyboardDeviceDriver::isPressed(KeyboardButton::KEY_W))		cameraMovement.z = -1;
		if (KeyboardDeviceDriver::isPressed(KeyboardButton::KEY_S))		cameraMovement.z = 1;
		if (KeyboardDeviceDriver::isPressed(KeyboardButton::KEY_A))		cameraMovement.x = -1;
		if (KeyboardDeviceDriver::isPressed(KeyboardButton::KEY_D))		cameraMovement.x = 1;
		if (KeyboardDeviceDriver::isPressed(KeyboardButton::KEY_Q))		cameraMovement.y = 1;
		if (KeyboardDeviceDriver::isPressed(KeyboardButton::KEY_Z))		cameraMovement.y = -1;
		if (glm::length(cameraMovement) < 0.00001f)
			cameraSpeed = 0;

		cameraMovement *= frameTime / 400.0f;
		if (KeyboardDeviceDriver::isModPressed(KeyboardModifiers::KEYMOD_SHIFT))
			cameraMovement *= 10.0f;

		cameraMovement *= cameraSpeed;
		cameraSpeed += (float)frameTime / 1000.0f;
		if (cameraSpeed > 1)
			cameraSpeed = 1;

		cameraPos += cameraMovement * cameraRot;

		cameraRot = cameraRot * glm::quat(glm::vec3(0, .01f * (mouseState.pos.x - lastMouseState.pos.x), 0));
		cameraRot = glm::quat(glm::vec3(.01f * (mouseState.pos.y - lastMouseState.pos.y), 0, 0)) * cameraRot;
		cameraRotTo = cameraRot;
	}
	cameraRot = glm::slerp(cameraRot, cameraRotTo, (float)frameTime / 100.0f);




	if (activeTool == EditTool::TRANSLATE)
	{
		glm::vec3 pos;

		auto targetPos = tien.scene.castRay(ray, false, [this](vrlib::tien::Node* n) {
			for (auto s : selectedNodes)
				if (n->isChildOf(s))
					return false;
			return true;// std::find(std::begin(selectedNodes), std::end(selectedNodes), n) == std::end(selectedNodes);
		});
		if (targetPos.first)
			pos = targetPos.second;
		else
		{
			vrlib::math::Plane plane(glm::vec3(0,1,0), 0);
			pos = plane.getCollisionPoint(ray);
		}

		if (isModPressed(KeyboardDeviceDriver::KEYMOD_SHIFT))
		{
			pos = glm::round(pos*2.0f)/2.0f;
		}
		if ((axis & Axis::X) == 0)
			pos.x = originalPosition.x;
		if ((axis & Axis::Y) == 0)
			pos.y = originalPosition.y;
		if ((axis & Axis::Z) == 0)
			pos.z = originalPosition.z;

		glm::vec3 diff = pos - getSelectionCenter();
		for(auto n : selectedNodes)
			n->transform->setGlobalPosition(n->transform->getGlobalPosition() + diff);
	}
	if (activeTool == EditTool::TRANSLATEWITHOUTCHILDREN)
	{
		glm::vec3 pos;

		auto targetPos = tien.scene.castRay(ray, false, [this](vrlib::tien::Node* n) {
			return std::find(std::begin(selectedNodes), std::end(selectedNodes), n) == std::end(selectedNodes);
		});
		if (targetPos.first)
			pos = targetPos.second;
		else
		{
			vrlib::math::Plane plane(glm::vec3(0, 1, 0), 0);
			pos = plane.getCollisionPoint(ray);
		}

		if (isModPressed(KeyboardDeviceDriver::KEYMOD_SHIFT))
		{
			pos = glm::round(pos*2.0f) / 2.0f;
		}
		if ((axis & Axis::X) == 0)
			pos.x = originalPosition.x;
		if ((axis & Axis::Y) == 0)
			pos.y = originalPosition.y;
		if ((axis & Axis::Z) == 0)
			pos.z = originalPosition.z;

		glm::vec3 diff = pos - getSelectionCenter();
		for (auto n : selectedNodes)
		{
			n->transform->position += diff;
			n->fortree([this, &n, &diff](const vrlib::tien::Node* nn)
			{
				if (nn->parent != n)
					return;
				nn->transform->position -= diff;
			});
		}
	}
	if (activeTool == EditTool::ROTATE || activeTool == EditTool::ROTATELOCAL)
	{
		glm::vec3 center = getSelectionCenter();
		float inc = 0.01f * glm::pi<float>() * (mouseState.pos.x - lastMouseState.pos.x);
		for (auto n : selectedNodes)
		{
			int axisIndex = axis == Axis::Z ? 2 : ((int)axis - 1);
			glm::vec3 rotationIncEuler;
			rotationIncEuler[axisIndex] = inc;


			if(activeTool == EditTool::ROTATELOCAL)
				n->transform->rotation = n->transform->rotation * glm::quat(rotationIncEuler);
			else
				n->transform->rotation = glm::quat(rotationIncEuler) * n->transform->rotation;


			glm::vec3 diff = n->transform->getGlobalPosition() - center;
			float len = glm::length(diff);
			if (len > 0.01)
			{
				float angle = glm::atan(axis == Axis::Z ? diff.y : diff.z, axis == Axis::X ? diff.y : diff.x);
				angle -= inc;
				if(axis == Axis::X)
					n->transform->setGlobalPosition(center + len * glm::vec3(0, glm::sin(angle), glm::cos(angle)));
				if (axis == Axis::Y)
					n->transform->setGlobalPosition(center + len * glm::vec3(glm::cos(angle), 0, glm::sin(angle)));
				if (axis == Axis::Z)
					n->transform->setGlobalPosition(center + len * glm::vec3(glm::cos(angle), glm::sin(angle), 0));
			}

			if (isModPressed(KeyboardModifiers::KEYMOD_SHIFT))
			{
				glm::vec3 euler = glm::degrees(glm::eulerAngles(n->transform->rotation));

				float diff = glm::radians(glm::round(euler[axisIndex] / 45.0f) * 45.0f - euler[axisIndex]);
				euler[axisIndex] = glm::round(euler[axisIndex] / 45.0f) * 45.0f;
				n->transform->rotation = glm::quat(glm::radians(euler));
					
				glm::vec3 diff2 = n->transform->position - center;
				float len = glm::length(diff2);
				if (len > 0.01)
				{ //wtf ok, this doesn't work?
					float angle = glm::atan(diff2.z, diff2.x);
					angle -= diff;
					n->transform->position = center + len * glm::vec3(glm::cos(angle), 0, glm::sin(angle));
				}

			}
		}
	}

	if (activeTool == EditTool::SCALE)
	{
		editorScale += mouseState.pos.x - lastMouseState.pos.x;
		glm::vec3 scale((axis & Axis::X) != 0 ? -1+glm::pow(2, 1 + editorScale / 100.0f) : 1,
						(axis & Axis::Y) != 0 ? -1 + glm::pow(2, 1 + editorScale / 100.0f) : 1,
						(axis & Axis::Z) != 0 ? -1 + glm::pow(2, 1 + editorScale / 100.0f) : 1);

		for (auto n : activeEditAction->actions)
		{
			auto a = dynamic_cast<NodeScaleAction*>(n);
			a->node->transform->scale = a->originalScale * scale;
		}
	}




	//if (mainPanel->components[1] == renderPanel)
		tien.update((float)(frameTime / 1000.0f));


	lastMouseState = mouseState;
}


void TienEdit::draw()
{
	float aspect = (float)kernel->getWindowWidth() / kernel->getWindowHeight();


	glViewport(0, 0, kernel->getWindowWidth(), kernel->getWindowHeight());
	menuOverlay.drawInit();
	menuOverlay.drawRootMenu();
	mainPanel->draw(&menuOverlay);
	menuOverlay.drawPopups();



	
	//if (mainPanel->components[1] == renderPanel)
	{
		glm::mat4 cameraMat = glm::translate(glm::toMat4(cameraRot), -cameraPos);
		glm::mat4 projectionMatrix = glm::perspective(70.0f, renderPanel->size.x / (float)renderPanel->size.y, 0.01f, 500.0f);
		glm::mat4 modelViewMatrix = cameraMat;
		glViewport(renderPanel->absPosition.x, kernel->getWindowHeight() - renderPanel->absPosition.y - renderPanel->size.y, renderPanel->size.x, renderPanel->size.y);
		tien.render(projectionMatrix, modelViewMatrix);

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(glm::value_ptr(projectionMatrix));
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glm::value_ptr(modelViewMatrix));
		glUseProgram(0);
		glDisable(GL_TEXTURE_2D);
		glColor4f(1, 0, 0, 1);
		glDisable(GL_BLEND);


		tien.scene.fortree([this](vrlib::tien::Node* n)
		{
			if (!n->transform)
				return;
			if (n->getComponent<vrlib::tien::components::Renderable>())
				return;

			float length = glm::pow(glm::distance(cameraPos, n->transform->getGlobalPosition()), 0.1f) / 3.0f;
			glPushMatrix();
			glMultMatrixf(glm::value_ptr(n->transform->globalTransform));
			glBegin(GL_LINES);
			glColor3f(1, 0, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(length, 0, 0);
			glColor3f(0, 1, 0);
			glVertex3f(0, 0, 0);
			glVertex3f(0, length, 0);
			glColor3f(0, 0, 1);
			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, length);
			glEnd();

			if (n->light)
			{
				if (n->light->type == vrlib::tien::components::Light::Type::directional)
				{
					glColor3f(1, 1, 0);
					glBegin(GL_LINES);
					for (float f = 0; f < glm::two_pi<float>(); f += glm::pi<float>() / 10)
					{
						glVertex3f(0, 0.25f*glm::cos(f), 0.25f*glm::sin(f));
						glVertex3f(1, 0.25f*glm::cos(f), 0.25f*glm::sin(f));
					}
					glEnd();
				}	
				else if (n->light->type == vrlib::tien::components::Light::Type::spot)
				{
					float dist = tan(glm::radians(n->light->spotlightAngle)/2.0f);
					glColor3f(1, 1, 0);
					glBegin(GL_LINES);
					for (float f = 0; f < glm::two_pi<float>(); f += glm::pi<float>() / 10)
					{
						glVertex3f(0, 0, 0);
						glVertex3f(1, dist*glm::cos(f), dist*glm::sin(f));
					}
					glEnd();
				}

			}




			glPopMatrix();
		});



		if(cacheSelection) //TODO: cache this differently, and draw this differently
		{ 
			if(selectionCache == 0)
				selectionCache = glGenLists(1);
			glNewList(selectionCache, GL_COMPILE);
			for (auto n : selectedNodes)
			{
				glPushMatrix();
				glMultMatrixf(glm::value_ptr(n->transform->globalTransform));
				{
					vrlib::tien::components::ModelRenderer* r = n->getComponent<vrlib::tien::components::ModelRenderer>();
					if (r)
					{
						glBegin(GL_LINES);
						auto triangles = r->model->getIndexedTriangles(); //TODO: cache this !
						for (size_t i = 0; i < triangles.first.size(); i += 3)
						{
							for (int ii = 0; ii < 3; ii++)
							{
								glVertex3fv(glm::value_ptr(triangles.second[triangles.first[i + ii]]));
								glVertex3fv(glm::value_ptr(triangles.second[triangles.first[i + (ii + 1) % 3]]));
							}
						}
						glEnd();
					}
				}
				{
					vrlib::tien::components::AnimatedModelRenderer* r = n->getComponent<vrlib::tien::components::AnimatedModelRenderer>();
					if (r && r->model)
					{
						glBegin(GL_LINES);
						auto triangles = r->model->getIndexedTriangles(); //TODO: cache this !
						for (size_t i = 0; i < triangles.first.size(); i += 3)
						{
							for (int ii = 0; ii < 3; ii++)
							{
								glVertex3fv(glm::value_ptr(triangles.second[triangles.first[i + ii]]));
								glVertex3fv(glm::value_ptr(triangles.second[triangles.first[i + (ii + 1) % 3]]));
							}
						}
						glEnd();
					}
				}
				{
					vrlib::tien::components::MeshRenderer* r = n->getComponent<vrlib::tien::components::MeshRenderer>();
					if (r && r->mesh)
					{
						glBegin(GL_LINES);
						for (size_t i = 0; i < r->mesh->indices.size(); i += 3)
						{
							for (int ii = 0; ii < 3; ii++)
							{
								glVertex3fv(glm::value_ptr(vrlib::gl::getP3(r->mesh->vertices[r->mesh->indices[i + ii]])));
								glVertex3fv(glm::value_ptr(vrlib::gl::getP3(r->mesh->vertices[r->mesh->indices[i + (ii+1)%3]])));
							}
						}
						glEnd();
					}
				}
				glPopMatrix();
			}
			glEndList();
			cacheSelection = false;
		}
		glDepthFunc(GL_LEQUAL);
		if(selectionCache > 0)
			glCallList(selectionCache);

		if (activeTool == EditTool::SCALE)
		{
			shader->use();
			shader->setUniform(EditorShaderUniforms::projectionMatrix, projectionMatrix);
			shader->setUniform(EditorShaderUniforms::textureFactor, 1.0f);
			shader->setUniform(EditorShaderUniforms::color, glm::vec4(1, 1, 1, 1));
			ruler->draw([&modelViewMatrix, this](const glm::mat4 &modelMatrix) {
				shader->setUniform(EditorShaderUniforms::modelViewMatrix, glm::translate(modelViewMatrix, getSelectionCenter()) * modelMatrix);

			}, [](const vrlib::Material& material)
			{
				material.texture->bind();
				return true;
			});
		}
	}



	glViewport(0, 0, menuOverlay.windowSize.x, menuOverlay.windowSize.y);
	menuOverlay.drawInit();
	if (activeTool == EditTool::SCALE)
	{
		menuOverlay.drawText("Scale: " + std::to_string(-1+glm::pow(2, 1+editorScale/100.0f)), glm::vec2(renderPanel->absPosition.x + 10, menuOverlay.windowSize.y - 12), glm::vec4(1, 1, 1, 1), true);
	}

	if (dragDrawCallback)
		dragDrawCallback(menuOverlay.mousePos);






}

void TienEdit::mouseMove(int x, int y)
{
	mouseState.pos.x = x;
	mouseState.pos.y = y;
	menuOverlay.mousePos = glm::vec2(x, y);

	if (renderPanel->inComponent(mouseState.pos))
	{
		glm::ivec2 mousePos = mouseState.pos;
		mousePos.y = kernel->getWindowHeight() - mousePos.y;
		glm::mat4 cameraMat = glm::translate(glm::toMat4(cameraRot), -cameraPos);
		glm::mat4 projection = glm::perspective(70.0f, renderPanel->size.x / (float)renderPanel->size.y, 0.01f, 500.0f);
		glm::vec4 Viewport(renderPanel->absPosition.x, kernel->getWindowHeight() - renderPanel->absPosition.y - renderPanel->size.y, renderPanel->size.x, renderPanel->size.y);
		glm::vec3 retNear = glm::unProject(glm::vec3(mousePos, 0), cameraMat, projection, glm::vec4(Viewport[0], Viewport[1], Viewport[2], Viewport[3]));
		glm::vec3 retFar = glm::unProject(glm::vec3(mousePos, 1), cameraMat, projection, glm::vec4(Viewport[0], Viewport[1], Viewport[2], Viewport[3]));
		ray = vrlib::math::Ray(retNear, glm::normalize(retFar - retNear));
	}

	if(focussedComponent && focussedComponent->inComponent(mouseState.mouseDownPos) && mouseState.left)
	{ 
		bool dragged = focussedComponent->mouseDrag(mouseState.left, mouseState.mouseDownPos, mouseState.pos);
		if (!dragged)
		{

		}
	}

}

void TienEdit::mouseScroll(int offset)
{
	bool scrolled = false;
	Component* c;
	if (renderPanel->inComponent(menuOverlay.mousePos))
	{
		cameraPos += glm::vec3(0, 0, -offset / 120.0f) * cameraRot;
	}
	//else if (focussedComponent) //TODO: find component under mouse to scroll
	//	scrolled |= focussedComponent->scroll(offset / 10.0f);
	if (!scrolled && (c = mainPanel->getComponentAt(menuOverlay.mousePos)))
	{
		scrolled |= c->scroll(offset / 10.0f);
		
		if (!scrolled)
			scrolled |= mainPanel->scrollRecursive(menuOverlay.mousePos, offset / 10.0f);


		if (scrolled)
		{
			if (focussedComponent)
			{
				focussedComponent->unfocus();
				focussedComponent->focussed = false;
			}
			focussedComponent = c;
			c->focus();
			c->focussed = true;
		}
	}
}


void TienEdit::mouseDown(MouseButton button)
{
	mouseState.mouseDownPos = mouseState.pos;
	mouseState.buttons[(int)button] = true;
	if (mouseState.middle)
		return;


	if (menuOverlay.click(button == vrlib::MouseButtonDeviceDriver::MouseButton::Left))
		return;

	Component* clickedComponent = mainPanel->getComponentAt(mouseState.pos);

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


	if (focussedComponent)
		focussedComponent->mouseDown(button == MouseButton::Left, mouseState.pos);
}

void TienEdit::keyDown(int button)
{
	NormalApp::keyDown(button);

	if (focussedComponent)
		if (focussedComponent->keyDown(button))
			return;
}

void TienEdit::keyUp(int button)
{
	NormalApp::keyUp(button);
	if (mouseState.middle)
		return;


	if (focussedComponent && focussedComponent != renderPanel)
	{
		if (focussedComponent->keyUp(button))
			return;
	}

	//if (focussedComponent == renderPanel)
	{
		if (buttonLookup[button] == KeyboardButton::KEY_G && activeTool != EditTool::TRANSLATE && activeTool != EditTool::TRANSLATEWITHOUTCHILDREN && !selectedNodes.empty())
		{
			activeTool = EditTool::TRANSLATE;
			originalPosition = getSelectionCenter();
			axis = Axis::XYZ;
		}
		else if (buttonLookup[button] == KeyboardButton::KEY_G && activeTool == EditTool::TRANSLATE)
		{
			activeTool = EditTool::TRANSLATEWITHOUTCHILDREN;
			glm::vec3 diff = originalPosition - getSelectionCenter();
			for (auto n : selectedNodes)
				n->transform->position += diff;
		}
		else if (buttonLookup[button] == KeyboardButton::KEY_G && activeTool == EditTool::TRANSLATEWITHOUTCHILDREN)
		{
			activeTool = EditTool::NONE;
			glm::vec3 diff = originalPosition - getSelectionCenter();
			for (auto n : selectedNodes)
				n->transform->position += diff;
		}
		if (buttonLookup[button] == KeyboardButton::KEY_R && activeTool != EditTool::ROTATE && activeTool != EditTool::ROTATELOCAL && !selectedNodes.empty())
		{
			activeTool = EditTool::ROTATE;
			originalPosition = getSelectionCenter();
			axis = Axis::Y;
			std::vector<Action*> actions;
			for (auto n : selectedNodes)
				actions.push_back(new NodeRotateAction(n));
			activeEditAction = new GroupAction(actions);
		}
		else if (buttonLookup[button] == KeyboardButton::KEY_R && activeTool == EditTool::ROTATE)
		{
			activeTool = EditTool::ROTATELOCAL;
			activeEditAction->undo(this);
		}
		else if(buttonLookup[button] == KeyboardButton::KEY_R && activeTool == EditTool::ROTATELOCAL)
		{
			activeTool = EditTool::NONE;
			activeEditAction->undo(this);
			delete activeEditAction;
			activeEditAction = nullptr;
		}

		if (buttonLookup[button] == KeyboardButton::KEY_S && activeTool != EditTool::SCALE && !selectedNodes.empty())
		{
			activeTool = EditTool::SCALE;
			originalPosition = getSelectionCenter();
			axis = Axis::XYZ;
			std::vector<Action*> actions;
			for (auto n : selectedNodes)
				actions.push_back(new NodeScaleAction(n));
			editorScale = 0;
			activeEditAction = new GroupAction(actions);
		}
		else if (buttonLookup[button] == KeyboardButton::KEY_S && activeTool == EditTool::SCALE)
		{
			activeTool = EditTool::NONE;
			activeEditAction->undo(this);
			delete activeEditAction;
			activeEditAction = nullptr;
		}

		if (buttonLookup[button] == KeyboardButton::KEY_X && activeTool != EditTool::NONE)
		{
			axis = isModPressed(KeyboardModifiers::KEYMOD_SHIFT) ? YZ : X;
			if (activeTool == EditTool::ROTATE || activeTool == EditTool::ROTATELOCAL || activeTool == EditTool::SCALE)
				activeEditAction->undo(this);
		}
		if (buttonLookup[button] == KeyboardButton::KEY_Y && activeTool != EditTool::NONE)
		{
			axis = isModPressed(KeyboardModifiers::KEYMOD_SHIFT) ? XZ : Y;
			if (activeTool == EditTool::ROTATE || activeTool == EditTool::ROTATELOCAL || activeTool == EditTool::SCALE)
				activeEditAction->undo(this);
		}
		if (buttonLookup[button] == KeyboardButton::KEY_Z && activeTool != EditTool::NONE)
		{
			axis = isModPressed(KeyboardModifiers::KEYMOD_SHIFT) ? XY : Z;
			if (activeTool == EditTool::ROTATE || activeTool == EditTool::ROTATELOCAL || activeTool == EditTool::SCALE)
				activeEditAction->undo(this);
		}


		if (activeTool == EditTool::NONE)
		{
			if (buttonLookup[button] == KeyboardButton::KEY_Z && isModPressed(KeyboardModifiers::KEYMOD_CTRLSHIFT))
				redo();
			else if (buttonLookup[button] == KeyboardButton::KEY_Z && isModPressed(KeyboardModifiers::KEYMOD_CTRL))
				undo();
			else if (buttonLookup[button] == KeyboardButton::KEY_C && isModPressed(KeyboardModifiers::KEYMOD_CTRL))
				copy();
			else if (buttonLookup[button] == KeyboardButton::KEY_V && isModPressed(KeyboardModifiers::KEYMOD_CTRL))
				paste();

			if (buttonLookup[button] == KeyboardButton::KEY_DELETE)
				deleteSelection();
		}


	}

}

void TienEdit::keyChar(char character)
{
	NormalApp::keyChar(character);
	if (mouseState.middle)
		return;
	if (focussedComponent && !mouseState.middle)
		focussedComponent->keyChar(character);
}


void TienEdit::mouseUp(MouseButton button)
{
	mouseState.buttons[(int)button] = false;
	if (mouseState.middle)
		return;

	if (button == vrlib::MouseButtonDeviceDriver::MouseButton::Left && glm::distance(glm::vec2(mouseState.mouseDownPos), glm::vec2(mouseState.pos)) < 3)
	{		//GetDoubleClickTime();
		DWORD time = GetTickCount();
		if (time - mouseState.lastClickTime < 250)
			mouseState.clickCount++;
		else
			mouseState.clickCount = 1;
		mouseState.lastClickTime = GetTickCount();
	}

	if(button != vrlib::MouseButtonDeviceDriver::MouseButton::Middle && glm::distance(glm::vec2(mouseState.mouseDownPos), glm::vec2(mouseState.pos)) < 3)
	{
		if (menuOverlay.click(button == vrlib::MouseButtonDeviceDriver::MouseButton::Left))
			return;

		Component* clickedComponent = mainPanel->getComponentAt(mouseState.pos);

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


		if (mainPanel->click(button == vrlib::MouseButtonDeviceDriver::MouseButton::Left, mouseState.pos, mouseState.clickCount))
			return;
	}

	if (button == vrlib::MouseButtonDeviceDriver::MouseButton::Left && glm::distance(glm::vec2(mouseState.mouseDownPos), glm::vec2(mouseState.pos)) < 3)
	{
		if (renderPanel->inComponent(mouseState.pos))
		{
			if (activeTool == EditTool::NONE)
			{
				vrlib::tien::Node* closestClickedNode = nullptr;
				float closest = 99999999.0f;
				tien.scene.castRay(ray, [&, this](vrlib::tien::Node* node, float hitFraction, const glm::vec3 &hitPosition, const glm::vec3 &hitNormal)
				{
					if (hitFraction < closest && hitFraction > 0)
					{
						closest = hitFraction;
						closestClickedNode = node;
					}
					return true;
				}, false);

				if (closestClickedNode != nullptr)
				{
					if (isModPressed(KeyboardModifiers::KEYMOD_SHIFT))
					{
						std::vector<vrlib::tien::Node*> newSelection = selectedNodes;
						if (std::find(std::begin(newSelection), std::end(newSelection), closestClickedNode) == std::end(newSelection))
							newSelection.push_back(closestClickedNode);
						else
							newSelection.erase(std::remove(newSelection.begin(), newSelection.end(), closestClickedNode), newSelection.end());
						perform(new SelectionChangeAction(this, newSelection ));
					}
					else
						perform(new SelectionChangeAction(this, { closestClickedNode }));
				}
				else
					perform(new SelectionChangeAction(this, {}));
			}
			else if (activeTool == EditTool::TRANSLATE)
			{
				glm::vec3 diff = originalPosition - getSelectionCenter();
				std::vector<Action*> group;
				for (auto n : selectedNodes)
					group.push_back(new NodeMoveAction(n, n->transform->position + diff, n->transform->position));
				actions.push_back(new GroupAction(group));
				activeTool = EditTool::NONE;
				cacheSelection = true;
				updateComponentsPanel(); //TODO: don't make it update all elements, but just the proper textboxes
			}
			else if (activeTool == EditTool::TRANSLATEWITHOUTCHILDREN)
			{ 
				glm::vec3 diff = originalPosition - getSelectionCenter();
				std::vector<Action*> group;
				for (auto n : selectedNodes)
					group.push_back(new NodeMoveAction(n, n->transform->position + diff, n->transform->position));
				actions.push_back(new GroupAction(group));
				activeTool = EditTool::NONE;
				cacheSelection = true;
				updateComponentsPanel(); //TODO: don't make it update all elements, but just the proper textboxes
			}
			else if (activeTool == EditTool::ROTATE || activeTool == EditTool::ROTATELOCAL)
			{
				activeTool = EditTool::NONE;
				for (Action* a : activeEditAction->actions)
				{
					auto sa = dynamic_cast<NodeRotateAction*>(a);
					sa->newRotation = sa->node->transform->rotation;
					sa->newPosition = sa->node->transform->position;
				}
				actions.push_back(activeEditAction);
				activeEditAction = nullptr;
				updateComponentsPanel(); //TODO: don't make it update all elements, but just the proper textboxes
				cacheSelection = true;
			}
			else if (activeTool == EditTool::SCALE)
			{
				activeTool = EditTool::NONE;
				for (Action* a : activeEditAction->actions)
				{
					auto sa = dynamic_cast<NodeScaleAction*>(a);
					sa->newScale = sa->node->transform->scale;
					sa->newPosition = sa->node->transform->position;
				}
				actions.push_back(activeEditAction);
				activeEditAction = nullptr;
				updateComponentsPanel(); //TODO: don't make it update all elements, but just the proper textboxes
				cacheSelection = true;
			}
		}
	}
	if (button == vrlib::MouseButtonDeviceDriver::Right && glm::distance(glm::vec2(mouseState.mouseDownPos), glm::vec2(mouseState.pos)) < 3)
	{
		if (activeTool == EditTool::NONE)
		{
			if (!selectedNodes.empty())
				perform(new SelectionChangeAction(this, {}));
		}
		else if (activeTool == EditTool::TRANSLATE)
		{
			activeTool = EditTool::NONE;
			glm::vec3 diff = originalPosition - getSelectionCenter();
			for (auto n : selectedNodes)
				n->transform->setGlobalPosition(n->transform->getGlobalPosition() + diff);
		}
		else if (activeTool == EditTool::ROTATE || activeTool == EditTool::ROTATELOCAL)
		{
			activeTool = EditTool::NONE;
			activeEditAction->undo(this);
			delete activeEditAction;
			activeEditAction = nullptr;
		}
		else if (activeTool == EditTool::SCALE)
		{
			activeTool = EditTool::NONE;
			activeEditAction->undo(this);
			delete activeEditAction;
			activeEditAction = nullptr;
		}
	}



//	if (glm::distance(glm::vec2(mouseState.mouseDownPos), glm::vec2(mouseState.pos)) >= 3)
	{
		if (focussedComponent && focussedComponent->inComponent(mouseState.mouseDownPos))
		{
			focussedComponent->mouseFinishDrag(button == MouseButton::Left, mouseState.mouseDownPos, mouseState.pos);
		}
	}

}


void TienEdit::perform(Action* action)
{
	action->perform(this);
	actions.push_back(action);

	for (auto a : redoactions)
		delete a;
	redoactions.clear();
}

glm::vec3 TienEdit::getSelectionCenter() const
{
	glm::vec3 center(0, 0, 0);
	for (auto n : selectedNodes)
		center += n->transform->getGlobalPosition() / (float)selectedNodes.size();
	return center;
}

void TienEdit::undo()
{
	if (actions.empty())
		return;
	Action* lastAction = actions.back();
	actions.pop_back();
	lastAction->undo(this);
	redoactions.push_back(lastAction);
}

void TienEdit::redo()
{
	if (redoactions.empty())
		return;
	Action* action = redoactions.back();
	redoactions.pop_back();
	action->perform(this);
	actions.push_back(action);
}

class ComponentRenderProps
{
public:
	bool folded = false;
};

std::map<vrlib::tien::Component*, ComponentRenderProps> renderProps;

void TienEdit::updateComponentsPanel()
{
	focussedComponent = nullptr;
	for (auto c : propertiesPanel->components)
		delete c;
	propertiesPanel->components.clear();
	if (objectTree->selectedItems.empty())
		return;
	vrlib::tien::Node* node = static_cast<vrlib::tien::Node*>(objectTree->selectedItems[0]);

	editorBuilder->reset();

	editorBuilder->addTitle("Node:");

	editorBuilder->beginGroup("Name");
	editorBuilder->addTextBox(node->name, [node, this](const std::string &newValue) { node->name = newValue; objectTree->update(); });
	editorBuilder->endGroup();

	editorBuilder->beginGroup("GUID");
	editorBuilder->addTextBox(node->guid, [node](const std::string &newValue) { node->guid = newValue; });
	editorBuilder->endGroup();

	editorBuilder->beginGroup("Parent");
	editorBuilder->addTextBox(node->parent ? node->parent->name : "-", [node](const std::string &newValue) {});
	editorBuilder->endGroup();

	std::vector<vrlib::tien::Component*> components = node->getComponents();
	for (auto c : components)
	{
		auto &props = renderProps[c];

		editorBuilder->addDivider();
		{
			Button* button = new Button("X", glm::ivec2(275, editorBuilder->line-5));
			button->size.x = 25;
			button->size.y = 20;
			button->onClick = [this,c, node]() {
				node->removeComponent(c);
				focussedComponent = nullptr;
				updateComponentsPanel();
			};
			propertiesPanel->components.push_back(button);
		}
		{
			Button* button = new Button(props.folded ? "\\\\/" : "/\\\\", glm::ivec2(250, editorBuilder->line-5));
			button->size.x = 25;
			button->size.y = 20;
			button->onClick = [&props, this]() { 
				props.folded = !props.folded; 
				focussedComponent = nullptr;
				updateComponentsPanel(); 
			};
			propertiesPanel->components.push_back(button);
		}
		c->buildEditor(editorBuilder, props.folded);
	}
	propertiesPanel->onReposition(mainPanel);
	editorBuilder->addDivider();

	editorBuilder->addTitle("");
	editorBuilder->beginGroup("Add Component");
	std::vector<std::string> keys;
	for (auto kv : componentFactory)
		keys.push_back(kv.first);
	vrlib::tien::EditorBuilder::TextComponent* comboBox = editorBuilder->addComboBox("-", keys, [](const std::string &newValue) {});

	editorBuilder->addButton("Add", [node, this, comboBox]() 
	{
		if (componentFactory.find(comboBox->getText()) == componentFactory.end())
			return;
		node->addComponent(componentFactory[comboBox->getText()].first(node)); //TODO: undo
		updateComponentsPanel();
	});
	editorBuilder->endGroup();
}



void TienEdit::save()
{
	char* filter = "All\0 * .*\0Scenes\0 * .json\0";
	char curdir[512];
	_getcwd(curdir, 512);

	std::string target = std::string(curdir) + "\\data\\virtueelpd\\scenes";

	HWND hWnd = GetActiveWindow();
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;

	char buf[256];
	ZeroMemory(buf, 256);
	strcpy(buf, "");
	ofn.lpstrFile = buf;
	ofn.nMaxFile = 256;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = target.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_ENABLESIZING;
	if (GetSaveFileName(&ofn))
	{
		_chdir(curdir);
		fileName = buf;
		std::replace(fileName.begin(), fileName.end(), '\\', '/');
		if (fileName.find(".") == std::string::npos)
			fileName += ".json";		

		vrlib::logger << "Save" << vrlib::Log::newline;
		vrlib::logger << "Saving to " << fileName;
		vrlib::json::Value saveFile;
		saveFile["meshes"] = vrlib::json::Value(vrlib::json::Type::arrayValue);
		saveFile["scene"] = tien.scene.asJson(saveFile["meshes"]);
		std::ofstream(fileName) << saveFile;
	}
	_chdir(curdir);

}

void TienEdit::load()
{
	char* filter = "All\0 * .*\0Scenes\0 * .json\0";
	char curdir[512];
	_getcwd(curdir, 512);

	std::string target = std::string(curdir) + "\\data\\virtueelpd\\scenes";

	HWND hWnd = GetActiveWindow();
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;

	char buf[256];
	ZeroMemory(buf, 256);
	strcpy(buf, fileName.c_str());
	ofn.lpstrFile = buf;
	ofn.nMaxFile = 256;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = target.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_ENABLESIZING;
	if (GetOpenFileName(&ofn))
	{
		_chdir(curdir);
		fileName = buf;
		std::replace(fileName.begin(), fileName.end(), '\\', '/');
		if (fileName.find(".") == std::string::npos)
			fileName += ".json";

		vrlib::logger << "Opening " << fileName << Log::newline;
		vrlib::json::Value saveFile = vrlib::json::readJson(std::ifstream(fileName));
		tien.scene.reset();
		tien.scene.fromJson(saveFile["scene"], saveFile, [this, saveFile](const vrlib::json::Value &json) -> vrlib::tien::Component*
		{
			std::string type = json["type"].asString();
			if (componentFactory.find(type) != componentFactory.end())
				return componentFactory[type].second(json, saveFile);
			type[0] = ::toupper(type[0]);
			if (componentFactory.find(type) != componentFactory.end())
				return componentFactory[type].second(json, saveFile);
			return nullptr;
		});
		selectedNodes.clear();
		objectTree->selectedItems = selectedNodes;
		objectTree->update();
	}
	_chdir(curdir);



}


void TienEdit::deleteSelection()
{
	vrlib::logger << "Delete" << vrlib::Log::newline;
	for (auto c : selectedNodes)
		delete c;
	selectedNodes.clear();
	objectTree->selectedItems.clear();
	objectTree->update();
	updateComponentsPanel();
	cacheSelection = true;
}


void toClipboard(const std::string &s) {
	OpenClipboard(nullptr);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size()+1);
	if (!hg) {
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), s.c_str(), s.size()+1);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
}

std::string fromClipboard()
{
	OpenClipboard(nullptr);
	HANDLE hData = GetClipboardData(CF_TEXT);
	assert(hData);
	char* pszText = static_cast<char*>(GlobalLock(hData));
	assert(pszText);
	std::string text(pszText);
	GlobalUnlock(hData);
	CloseClipboard();
	return text;
}

void TienEdit::copy()
{
	vrlib::logger << "Copy" << vrlib::Log::newline;

	vrlib::json::Value clipboard;	
	clipboard["nodes"] = vrlib::json::Value(vrlib::json::Type::arrayValue);
	for (auto c : selectedNodes)
	{
		// TODO: only copy parents, not children of selected parents
		clipboard["nodes"].push_back(c->asJson(clipboard["meshes"]));
	}

	std::string out;
	out << clipboard;
	toClipboard(out);
}

void TienEdit::paste()
{
	vrlib::logger << "Paste" << vrlib::Log::newline;

	vrlib::json::Value clipboard = vrlib::json::readJson(std::stringstream(fromClipboard()));
	if (clipboard.isNull())
	{
		vrlib::logger << "Invalid json on clipboard" << vrlib::Log::newline;
		return;
	}

	selectedNodes.clear();
	for (const vrlib::json::Value &n : clipboard["nodes"])
	{
		vrlib::tien::Node* newNode = new vrlib::tien::Node("", &tien.scene);
		newNode->fromJson(n, clipboard);

		newNode->fortree([](vrlib::tien::Node* n) {
			n->guid = vrlib::util::getGuid();
		});

		selectedNodes.push_back(newNode);
	}

	objectTree->selectedItems = selectedNodes;
	objectTree->update();

	activeTool = EditTool::TRANSLATE;
	originalPosition = getSelectionCenter();
	axis = Axis::XYZ;

}


void TienEdit::focusSelectedObject()
{
	glm::vec3 lookat = getSelectionCenter();

	glm::mat4 mat = glm::lookAt(cameraPos, lookat, glm::vec3(0, 1, 0));
	cameraRotTo = glm::quat(mat);
}


void TienEdit::sortScene()
{
	tien.scene.fortree([](vrlib::tien::Node* node)
	{
		node->sortChildren();
	});

	std::set<std::string> ids;
	tien.scene.fortree([&ids](vrlib::tien::Node* node)
	{
		if (ids.find(node->guid) != ids.end())
			node->guid = vrlib::util::getGuid();
		ids.insert(node->guid);
	});





	objectTree->update();
}