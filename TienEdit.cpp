#define _CRT_SECURE_NO_WARNINGS
#include "TienEdit.h"
#include "resource.h"

#include <Windows.h>
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
#include <VrLib/tien/components/postprocess/Bloom.h>
#include <VrLib/tien/components/postprocess/Gamma.h>
#include <VrLib/tien/components/postprocess/DoF.h>

#include "menu/MenuOverlay.h"
#include "menu/Menu.h"
#include "menu/ToggleMenuItem.h"
#include "wm/SplitPanel.h"
#include "wm/RenderComponent.h"
#include "wm/Tree.h"
#include "wm/Panel.h"
#include "wm/Label.h"
#include "wm/Image.h"
#include "wm/Button.h"
#include "wm/ScrollPanel.h"
#include "wm/ComboBox.h"

#include "actions/SelectionChangeAction.h"
#include "actions/GroupAction.h"
#include "actions/NodeMoveAction.h"
#include "actions/NodeScaleAction.h"
#include "actions/NodeRotateAction.h"

#include "StubComponent.h"
#include "EditorBuilderGui.h"
#include "BrowsePanel.h"


#define SCENEWIDTH 300
#define PROPERTYWIDTH 300
#define BROWSERHEIGHT 300
#define BROWSERTOOLBARHEIGHT 32

MouseState TienEdit::mouseState;
MouseState TienEdit::lastMouseState;



typedef std::pair<std::function<vrlib::tien::Component*(vrlib::tien::Node*)>, std::function<vrlib::tien::Component*(const json&, const json&)>> ComponentPair;
std::map<std::string, ComponentPair> componentFactory =
{
	{ "Mesh Renderer", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::MeshRenderer(); }, 
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::MeshRenderer(data, totalJson); }) },
	{ "Model Renderer", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::ModelRenderer(""); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::ModelRenderer(data); }) },
	{ "Animated Model Renderer", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::AnimatedModelRenderer(""); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::AnimatedModelRenderer(data); }) },
	{ "Terrain Renderer", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::TerrainRenderer(nullptr); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::TerrainRenderer(nullptr); }) },
	{ "Light", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::Light(); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::Light(data); }) },
	{ "Static Skybox", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::StaticSkyBox(); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::StaticSkyBox(); }) },
	{ "Dynamic Skybox", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::DynamicSkyBox(); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::DynamicSkyBox(data); }) },
	{ "Camera", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::Camera(); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::Camera(data); }) },
	{ "RigidBody", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::RigidBody(0.0f, vrlib::tien::components::RigidBody::Type::Static); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::RigidBody(data); }) },
	{ "BoxCollider", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::BoxCollider(n); },
		[](const json &data, const json &totalJson) { return vrlib::tien::components::BoxCollider::fromJson(data, nullptr); }) },
	{ "MeshCollider", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::MeshCollider(n, true); },
		[](const json &data, const json &totalJson) { throw "ugh";  return new vrlib::tien::components::MeshCollider(nullptr, true); }) },
	{ "SphereCollider", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::SphereCollider(); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::SphereCollider(); }) },
	{ "TerrainCollider", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::TerrainCollider(n); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::TerrainCollider(); }) },
	{ "PostProcessing Bloom", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::postprocessors::Bloom(); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::postprocessors::Bloom(); }) },
	{ "PostProcessing Gamma", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::postprocessors::Gamma(); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::postprocessors::Gamma(); }) },
	{ "PostProcessing Depth of Field", ComponentPair(
		[](vrlib::tien::Node* n) { return new vrlib::tien::components::postprocessors::DoF(); },
		[](const json &data, const json &totalJson) { return new vrlib::tien::components::postprocessors::DoF(); }) },


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
	std::ifstream file("data/TiEnEdit/stubs.json");
	json stubs;
	file >> stubs;
	for (const json &stub : stubs)
	{
		componentFactory[stub["name"]] = ComponentPair([stub](vrlib::tien::Node* n)
		{
			StubComponent* c = new StubComponent();
			c->stub = stub;
			return c;
		},
		[stub](const json &data, const json &totalJson)
		{
			StubComponent* c = new StubComponent(data);
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

	vrlib::tien::_tieneditor = true;
	tien.init();

	//initialize menus
	menuOverlay.init();
	menuOverlay.loadMenu("data/TiEnEdit/menu.json");
	menuOverlay.rootMenu->setAction("file/new", std::bind(&TienEdit::newScene, this));
	menuOverlay.rootMenu->setAction("file/open", std::bind(&TienEdit::load, this));
	menuOverlay.rootMenu->setAction("file/save", std::bind(&TienEdit::save, this));

	menuOverlay.rootMenu->setAction("edit/undo", std::bind(&TienEdit::undo, this));
	menuOverlay.rootMenu->setAction("edit/redo", std::bind(&TienEdit::redo, this));
	menuOverlay.rootMenu->setAction("edit/sort", std::bind(&TienEdit::sortScene, this));

	menuOverlay.rootMenu->setAction("object/copy", std::bind(&TienEdit::copy, this));
	menuOverlay.rootMenu->setAction("object/paste", std::bind(&TienEdit::paste, this));
	menuOverlay.rootMenu->setAction("object/delete", std::bind(&TienEdit::deleteSelection, this));


	menuOverlay.addToolbarButton(0, "New", std::bind(&TienEdit::newScene, this));
	menuOverlay.addToolbarButton(1, "Open", std::bind(&TienEdit::load, this));
	menuOverlay.addToolbarButton(2, "Save", std::bind(&TienEdit::save, this));
	menuOverlay.addToolbarSeperator();
	menuOverlay.addToolbarButton(3, "Undo", std::bind(&TienEdit::undo, this));
	menuOverlay.addToolbarButton(4, "Redo", std::bind(&TienEdit::redo, this));
	menuOverlay.addToolbarSeperator();
	menuOverlay.addToolbarButton(5, "Grab", []() {});
	menuOverlay.addToolbarButton(6, "Rotate", []() {});
	menuOverlay.addToolbarButton(7, "Scale", []() {});

	debugPhysics = dynamic_cast<ToggleMenuItem*>(menuOverlay.rootMenu->getItem("View/Physics meshes"));
	debugCamera = dynamic_cast<ToggleMenuItem*>(menuOverlay.rootMenu->getItem("View/Preview selected camera"));


	//build up UI
	menuOverlay.mainPanel = mainPanel = new SplitPanel(SplitPanel::Alignment::HORIZONTAL);
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

	SplitPanel* browseSplitter = new SplitPanel(SplitPanel::Alignment::VERTICAL);
	subPanel->addPanel(browseSplitter);
	browseSplitter->addPanel(browseToolbar.panel = new Panel());
	browseSplitter->addPanel(new ScrollPanel(browsePanel = new BrowsePanel(this)));
	browseSplitter->sizes[0] = BROWSERTOOLBARHEIGHT;

	subPanel->sizes[1] = BROWSERHEIGHT + BROWSERTOOLBARHEIGHT;
	subPanel->sizes[0] = mainPanel->size.y - BROWSERHEIGHT - BROWSERTOOLBARHEIGHT;

	mainPanel->addPanel(subPanel);
	propertiesPanel = new Panel();
	propertiesPanel->size.x = PROPERTYWIDTH;
	propertiesPanel->size.y = 100000;
	mainPanel->addPanel(new ScrollPanel( propertiesPanel ));


	browseToolbar.panel->components.push_back(browseToolbar.typeFilter = new ComboBox("all", glm::ivec2(5, 5)));
	browseToolbar.typeFilter->size = glm::ivec2(100, 25);
	browseToolbar.typeFilter->values = { "all", "models", "textures", "prefabs" };
	browseToolbar.typeFilter->onChange = [this]() {browsePanel->rebuild(browsePanel->directory);  };

	browseToolbar.panel->components.push_back(browseToolbar.searchFilter = new TextField("", glm::ivec2(105, 5)));
	browseToolbar.searchFilter->emptyValue = "Search...";
	browseToolbar.searchFilter->size = glm::ivec2(200, 25);
	browseToolbar.searchFilter->onChange = [this]() {
		static DelayedTask keyTask = 0;
		if (keyTask != 0)
			stopDelayed(keyTask);
		keyTask = runDelayed(0.5, [this]()
		{
			browsePanel->rebuild(browsePanel->directory); 
			menuOverlay.focussedComponent = browseToolbar.searchFilter;
			keyTask = 0;
		});
	};

	browseToolbar.panel->components.push_back(browseToolbar.directory = new Label("/data", glm::ivec2(305, 5)));
	browseToolbar.directory->size = glm::ivec2(200, 25);
	
	browsePanel->rebuild("./data/models/");


	editorBuilder = new GuiEditor(this, propertiesPanel);

	objectTree->rightClickItem = [this]()
	{
		if (!objectTree->selectedItems.empty())
		{
			Menu* menu = Menu::load("data/TiEnEdit/nodemenu.json");
			menuOverlay.popupMenus.push_back(std::pair<glm::vec2, Menu*>(mouseState.pos, menu));
			menu->setAction("duplicate", std::bind(&TienEdit::duplicate, this));
			menu->setAction("delete", std::bind(&TienEdit::deleteSelection, this));
			menu->setAction("focus with camera", std::bind(&TienEdit::focusSelectedObject, this));
			menu->setAction("add debug", [this]() { for (auto i : objectTree->selectedItems) { i->addDebugChildSphere(); } });
			menu->setAction("make prefab", [this]() {
				//TODO: check if the currently selected node is already a prefab
				menuOverlay.showInputDialog("Please enter prefab name", browsePanel->directory + objectTree->selectedItems[0]->name + ".json", [this](const std::string value)
				{
					json prefabJson;
					prefabJson["nodes"] = json::array();
					for (auto c : objectTree->selectedItems)
						prefabJson["nodes"].push_back(c->asJson(prefabJson["meshes"]));
					std::ofstream pFile(value.c_str());
					pFile << prefabJson.dump();
				});
			});
			if (objectTree->selectedItems.size() > 1)
			{
				menu->getItem("csg")->enabled = true;
				menu->setAction("csg/union", std::bind(&TienEdit::csgOperate, this, dynamic_cast<ToggleMenuItem*>(menu->getItem("csg/keep old"))->getValue(), CsgOp::Union));
				menu->setAction("csg/difference", std::bind(&TienEdit::csgOperate, this, dynamic_cast<ToggleMenuItem*>(menu->getItem("csg/keep old"))->getValue(), CsgOp::Difference));
				menu->setAction("csg/intersect", std::bind(&TienEdit::csgOperate, this, dynamic_cast<ToggleMenuItem*>(menu->getItem("csg/keep old"))->getValue(), CsgOp::Intersect));
			}
			else
			{
				menu->getItem("csg")->enabled = false;
			}
		}
		else
		{
			Menu* menu = Menu::load("data/TiEnEdit/newnodemenu.json");
			menuOverlay.popupMenus.push_back(std::pair<glm::vec2, Menu*>(mouseState.pos, menu));
			menu->setAction("new node", [this]()
			{
				vrlib::tien::Node* n = new vrlib::tien::Node("new node", &tien.scene);
				n->addComponent(new vrlib::tien::components::Transform());
				perform(new SelectionChangeAction(this, { n }));
			});
			menu->setAction("new cube", std::bind(&TienEdit::newMesh, this, "cube", new vrlib::tien::components::MeshRenderer::Cube()));
			menu->setAction("new plane", std::bind(&TienEdit::newMesh, this, "plane", new vrlib::tien::components::MeshRenderer::Plane()));
			menu->setAction("new sphere", std::bind(&TienEdit::newMesh, this, "sphere", new vrlib::tien::components::MeshRenderer::Sphere()));
		}
	};
	objectTree->selectItem = [this]()
	{
		perform(new SelectionChangeAction(this, objectTree->selectedItems));
	};
	objectTree->doubleClickItem = std::bind(&TienEdit::focusSelectedObject, this);
	objectTree->dragItem = [this](std::vector<vrlib::tien::Node*> from, vrlib::tien::Node* to)
	{//TODO: undo
		if (!to)
			to = &tien.scene;
		for(auto n : from)
			n->setParent(to); //TODO: keep transforms
		objectTree->update();
	};



	mainPanel->position = glm::ivec2(0, 25+36);
	mainPanel->size = glm::ivec2(kernel->getWindowWidth(), kernel->getWindowHeight());
	mainPanel->sizes[0] = SCENEWIDTH;
	mainPanel->sizes[2] = PROPERTYWIDTH;
	mainPanel->sizes[1] = mainPanel->size.x - SCENEWIDTH-PROPERTYWIDTH;
	dynamic_cast<SplitPanel*>(mainPanel->components[1])->sizes[0] = mainPanel->size.y - BROWSERHEIGHT - BROWSERTOOLBARHEIGHT;
	mainPanel->onReposition(nullptr);

	vrlib::tien::Node* sunlight;
	{
		vrlib::tien::Node* n = new vrlib::tien::Node("Sunlight", &tien.scene);
		n->addComponent(new vrlib::tien::components::Transform(glm::vec3(0, 4, 0), glm::quat(glm::vec3(1,1,1))));
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


	menuOverlay.focussedComponent = renderPanel;

	objectTree->update();


	cameraPos = glm::vec3(0, 1.8f, 8.0f);
	load();
	tien.start();
	tien.update(0.000001f);
	tien.pause();

}

void TienEdit::preFrame(double frameTime, double totalTime)
{
	//first reposition the overlay menu
	menuOverlay.setWindowSize(kernel->getWindowSize());
	menuOverlay.hover();
	
	//handle the layout for the main panel
	mainPanel->size = glm::ivec2(kernel->getWindowWidth(), kernel->getWindowHeight() - 25 - 36);
	mainPanel->sizes[1] = mainPanel->size.x - 600;
	mainPanel->onReposition(nullptr); //TODO: don't do this every frame ._.;

	//camera movement
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
	//slowly interpolate camera movement
	cameraRot = glm::slerp(cameraRot, cameraRotTo, (float)frameTime / 100.0f);



	//handle tools....
	if (activeTool == EditTool::TRANSLATE)
	{
		glm::vec3 pos;

		auto targetPos = tien.scene.castRay(ray, false, [this](vrlib::tien::Node* n) {
			for (auto s : objectTree->selectedItems)
				if (n->isChildOf(s))
					return false;
			return true;// std::find(std::begin(objectTree->selectedItems), std::end(objectTree->selectedItems), n) == std::end(objectTree->selectedItems);
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
		for(auto n : objectTree->selectedItems)
			n->transform->setGlobalPosition(n->transform->getGlobalPosition() + diff);
		rebakeSelectedLights();
	}
	if (activeTool == EditTool::TRANSLATEWITHOUTCHILDREN)
	{
		glm::vec3 pos;

		auto targetPos = tien.scene.castRay(ray, false, [this](vrlib::tien::Node* n) {
			return std::find(std::begin(objectTree->selectedItems), std::end(objectTree->selectedItems), n) == std::end(objectTree->selectedItems);
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
		for (auto n : objectTree->selectedItems)
		{
			n->transform->position += diff;
			n->fortree([this, &n, &diff](const vrlib::tien::Node* nn)
			{
				if (nn->parent != n)
					return;
				nn->transform->position -= diff;
			});
		}
		rebakeSelectedLights();
	}
	if (activeTool == EditTool::ROTATE || activeTool == EditTool::ROTATELOCAL)
	{
		glm::vec3 center = getSelectionCenter();
		float inc = 0.01f * glm::pi<float>() * (mouseState.pos.x - lastMouseState.pos.x);
		for (auto n : objectTree->selectedItems)
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
			rebakeSelectedLights();
		}
	}

	if (activeTool == EditTool::SCALE)
	{
		editorScale += mouseState.pos.x - lastMouseState.pos.x;
		if (editorScale < -99)
			editorScale = -99;
		glm::vec3 scale((axis & Axis::X) != 0 ? -1+glm::pow(2, 1 + editorScale / 100.0f) : 1,
						(axis & Axis::Y) != 0 ? -1 + glm::pow(2, 1 + editorScale / 100.0f) : 1,
						(axis & Axis::Z) != 0 ? -1 + glm::pow(2, 1 + editorScale / 100.0f) : 1);

		for (auto n : activeEditAction->actions)
		{
			auto a = dynamic_cast<NodeScaleAction*>(n);
			a->node->transform->scale = a->originalScale * scale;
		}
	}


	for (auto it = delayedTasks.begin(); it != delayedTasks.end();)
	{
		std::get<0>(it->second) -= frameTime / 1000.0f;
		if (std::get<0>(it->second) <= 0)
		{
			std::get<1>(it->second)();
			it = delayedTasks.erase(it);
		}
		else
			it++;
	}
	


	//update tien, though it is paused
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
	if(menuOverlay.focussedComponent)
		menuOverlay.focussedComponent->draw(&menuOverlay);
	menuOverlay.drawPopups();



	
	//if (mainPanel->components[1] == renderPanel)
	{
		glm::mat4 cameraMat = glm::translate(glm::toMat4(cameraRot), -cameraPos);
		glm::mat4 projectionMatrix = glm::perspective(70.0f, renderPanel->size.x / (float)renderPanel->size.y, 0.01f, 500.0f);
		glm::mat4 modelViewMatrix = cameraMat;
		glViewport(renderPanel->absPosition.x, kernel->getWindowHeight() - renderPanel->absPosition.y - renderPanel->size.y, renderPanel->size.x, renderPanel->size.y);

		tien.scene.cameraNode = nullptr;
		if (debugCamera->getValue())
			for (auto n : objectTree->selectedItems)
				if (n->getComponent<vrlib::tien::components::Camera>())
					tien.scene.cameraNode = n;

		tien.render(projectionMatrix, modelViewMatrix);

		// Set proper matrices
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(glm::value_ptr(projectionMatrix));
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glm::value_ptr(modelViewMatrix));
		glUseProgram(0);
		glDisable(GL_TEXTURE_2D);
		glColor4f(1, 0, 0, 1);
		glDisable(GL_BLEND);

		drawSelectionOrigins();
		drawSelection();

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


	if (debugPhysics->getValue())
		drawPhysicsDebug();


	glViewport(0, 0, menuOverlay.windowSize.x, menuOverlay.windowSize.y);
	menuOverlay.drawInit();
	if (activeTool == EditTool::SCALE)
	{
		menuOverlay.drawText("Scale: " + std::to_string(-1+glm::pow(2, 1+editorScale/100.0f)), glm::vec2(renderPanel->absPosition.x + 10, menuOverlay.windowSize.y - 12), glm::vec4(1, 1, 1, 1), true);
	}
	menuOverlay.drawPopups();

	if (dragDrawCallback)
		dragDrawCallback(menuOverlay.mousePos);
}

void TienEdit::drawSelectionOrigins()
{
	tien.scene.fortree([this](vrlib::tien::Node* n)
	{
		if (!n->transform)
			return;
		if (n->getComponent<vrlib::tien::components::Renderable>())
			return;

		float length = glm::pow(glm::distance(cameraPos, n->transform->getGlobalPosition()), 0.1f) / 3.0f;
		glPushMatrix();
		glMultMatrixf(glm::value_ptr(n->transform->globalTransform));
		glLineWidth(1);
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
				if (std::find(objectTree->selectedItems.begin(), objectTree->selectedItems.end(), n) == objectTree->selectedItems.end())
					glColor3f(1, 1, 0);
				else
					glColor3f(1, 0, 0);
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
				float dist = tan(glm::radians(n->light->spotlightAngle) / 2.0f);
				if (std::find(objectTree->selectedItems.begin(), objectTree->selectedItems.end(), n) == objectTree->selectedItems.end())
					glColor3f(1, 1, 0);
				else
					glColor3f(1, 0, 0);
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
}

void TienEdit::drawSelection()
{
	if (cacheSelection) //TODO: cache this differently, and draw this differently
	{
		if (selectionCache == 0)
			selectionCache = glGenLists(1);
		glNewList(selectionCache, GL_COMPILE);
		glLineWidth(1);
		for (auto n : objectTree->selectedItems)
		{
			if (!n || !n->transform)
				continue;
			glPushMatrix();
			glMultMatrixf(glm::value_ptr(n->transform->globalTransform));
			{
				vrlib::tien::components::ModelRenderer* r = n->getComponent<vrlib::tien::components::ModelRenderer>();
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
							glVertex3fv(glm::value_ptr(vrlib::gl::getP3(r->mesh->vertices[r->mesh->indices[i + (ii + 1) % 3]])));
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
	if (selectionCache > 0)
		glCallList(selectionCache);
}

void TienEdit::drawPhysicsDebug()
{
	glColor4f(1, 0, 0, 1);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	tien.scene.fortree([](const vrlib::tien::Node* n)
	{
		auto colliders = n->getComponents<vrlib::tien::components::Collider>();
		for (auto collider : colliders)
			collider->drawDebug();
	});
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1, 0, 0, 0.1f);
	tien.scene.fortree([](const vrlib::tien::Node* n)
	{
		auto colliders = n->getComponents<vrlib::tien::components::Collider>();
		for (auto collider : colliders)
			collider->drawDebug();
	});
	glDisable(GL_CULL_FACE);
}



void TienEdit::mouseMove(int x, int y)
{
	mouseState.pos.x = x;
	mouseState.pos.y = y;
	menuOverlay.mouseMove(glm::vec2(x, y));

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
}

void TienEdit::mouseScroll(int offset)
{
	bool scrolled = false;
	if (renderPanel->inComponent(menuOverlay.mousePos))
		cameraPos += glm::vec3(0, 0, -offset / 120.0f) * cameraRot;
	else if (!scrolled)
		scrolled |= menuOverlay.mouseScroll(offset);
	//else if (focussedComponent) //TODO: find component under mouse to scroll
	//	scrolled |= focussedComponent->scroll(offset / 10.0f);
}


void TienEdit::mouseDown(MouseButton button)
{
	mouseState.mouseDownPos = mouseState.pos;
	mouseState.buttons[(int)button] = true;
	if (mouseState.middle)
		return;

	if (menuOverlay.click(button == vrlib::MouseButtonDeviceDriver::MouseButton::Left))
		return;


}

void TienEdit::keyDown(int button)
{
	NormalApp::keyDown(button);

	if (menuOverlay.focussedComponent)
		if (menuOverlay.focussedComponent->keyDown(button))
			return;
}

void TienEdit::keyUp(int button)
{
	NormalApp::keyUp(button);
	if (mouseState.middle)
		return;


	if (menuOverlay.focussedComponent && menuOverlay.focussedComponent != renderPanel)
	{
		if (menuOverlay.focussedComponent->keyUp(button))
			return;
	}

	//if (focussedComponent == renderPanel)
	{
		if (buttonLookup[button] == KeyboardButton::KEY_G && activeTool != EditTool::TRANSLATE && activeTool != EditTool::TRANSLATEWITHOUTCHILDREN && !objectTree->selectedItems.empty())
		{
			if (activeTool != EditTool::NONE)
				finishCurrentTransformAction();
			activeTool = EditTool::TRANSLATE;
			originalPosition = getSelectionCenter();
			axis = Axis::XYZ;
		}
		else if (buttonLookup[button] == KeyboardButton::KEY_G && activeTool == EditTool::TRANSLATE)
		{
			if (activeTool != EditTool::NONE && activeTool != EditTool::TRANSLATE)
				finishCurrentTransformAction();
			activeTool = EditTool::TRANSLATEWITHOUTCHILDREN;
			glm::vec3 diff = originalPosition - getSelectionCenter();
			for (auto n : objectTree->selectedItems)
				n->transform->position += diff;
		}
		else if (buttonLookup[button] == KeyboardButton::KEY_G && activeTool == EditTool::TRANSLATEWITHOUTCHILDREN)
		{
			activeTool = EditTool::NONE;
			glm::vec3 diff = originalPosition - getSelectionCenter();
			for (auto n : objectTree->selectedItems)
				n->transform->position += diff;
		}
		if (buttonLookup[button] == KeyboardButton::KEY_R && activeTool != EditTool::ROTATE && activeTool != EditTool::ROTATELOCAL && !objectTree->selectedItems.empty())
		{
			if (activeTool != EditTool::NONE)
				finishCurrentTransformAction();
			activeTool = EditTool::ROTATE;
			originalPosition = getSelectionCenter();
			axis = Axis::Y;
			std::vector<Action*> actions;
			for (auto n : objectTree->selectedItems)
				actions.push_back(new NodeRotateAction(n));
			activeEditAction = new GroupAction(actions);
		}
		else if (buttonLookup[button] == KeyboardButton::KEY_R && activeTool == EditTool::ROTATE)
		{
			if (activeTool != EditTool::NONE && activeTool != EditTool::ROTATE)
				finishCurrentTransformAction();
			activeTool = EditTool::ROTATELOCAL;
			activeEditAction->undo(this);
		}
		else if(buttonLookup[button] == KeyboardButton::KEY_R && activeTool == EditTool::ROTATELOCAL)
		{
			if (activeTool != EditTool::NONE)
				finishCurrentTransformAction();
			activeTool = EditTool::NONE;
			if (activeEditAction)
			{
				activeEditAction->undo(this);
				delete activeEditAction;
			}
			activeEditAction = nullptr;
		}

		if (buttonLookup[button] == KeyboardButton::KEY_S && activeTool != EditTool::SCALE && !objectTree->selectedItems.empty())
		{
			if (activeTool != EditTool::NONE)
				finishCurrentTransformAction();
			activeTool = EditTool::SCALE;
			originalPosition = getSelectionCenter();
			axis = Axis::XYZ;
			std::vector<Action*> actions;
			for (auto n : objectTree->selectedItems)
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
	//TODO: move to menuOverlay
	if (menuOverlay.focussedComponent && !mouseState.middle)
	{
		if (character == '\t' && menuOverlay.focussedComponent->focusable)
		{
			std::function<void(Component*)> nextComponent;
			nextComponent = [this, &nextComponent](Component* c)
			{
				Panel* p = dynamic_cast<Panel*>(c);
				if (p)
				{
					for (size_t i = 0; i < p->components.size(); i++)
					{
						if (p->components[i] == menuOverlay.focussedComponent)
						{
							menuOverlay.focussedComponent->unfocus();
							menuOverlay.focussedComponent->focussed = false;
							do
							{
								i = (i + 1) % p->components.size();
								menuOverlay.focussedComponent = p->components[i];
							} while (!menuOverlay.focussedComponent->focusable);
						
							menuOverlay.focussedComponent->focussed = true;
							menuOverlay.focussedComponent->focus();
							break;
						}
						else
							nextComponent(p->components[i]);
					}
				}

				if (dynamic_cast<ScrollPanel*>(c))
					nextComponent(dynamic_cast<ScrollPanel*>(c)->component);
			};
			nextComponent(menuOverlay.getRootComponent());			
		}
		else
			menuOverlay.focussedComponent->keyChar(character);
	}
}

void TienEdit::updateNodePointer(vrlib::tien::Node * oldNode, vrlib::tien::Node * newNode)
{
	tien.scene.updateNodePointer(oldNode, newNode);
	for (auto a : actions)
		a->updateNodePointer(oldNode, newNode);
}

bool TienEdit::checkNodePointer(vrlib::tien::Node * oldNode)
{
	bool ret = false;
	tien.scene.fortree([&ret](const vrlib::tien::Node* n)
	{
		
	});
	return ret;
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
		if (menuOverlay.wasClicked())
			return;

		if (menuOverlay.mouseUp(button == vrlib::MouseButtonDeviceDriver::MouseButton::Left))
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
						std::vector<vrlib::tien::Node*> newSelection = objectTree->selectedItems;
						if (std::find(std::begin(newSelection), std::end(newSelection), closestClickedNode) == std::end(newSelection))
							newSelection.push_back(closestClickedNode);
						else
							newSelection.erase(std::remove(newSelection.begin(), newSelection.end(), closestClickedNode), newSelection.end());
						perform(new SelectionChangeAction(this, newSelection));
					}
					else
						perform(new SelectionChangeAction(this, { closestClickedNode }));
				}
				else
					perform(new SelectionChangeAction(this, {}));
			}
			else
				finishCurrentTransformAction();
				
		}
	}
	if (button == vrlib::MouseButtonDeviceDriver::Right && glm::distance(glm::vec2(mouseState.mouseDownPos), glm::vec2(mouseState.pos)) < 3 && renderPanel->inComponent(mouseState.mouseDownPos))
	{
		if (activeTool == EditTool::NONE)
		{
			if (!objectTree->selectedItems.empty())
				perform(new SelectionChangeAction(this, {}));
		}
		else if (activeTool == EditTool::TRANSLATE)
		{
			activeTool = EditTool::NONE;
			glm::vec3 diff = originalPosition - getSelectionCenter();
			for (auto n : objectTree->selectedItems)
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
		if (menuOverlay.focussedComponent && menuOverlay.focussedComponent->inComponent(mouseState.mouseDownPos))
		{
			menuOverlay.focussedComponent->mouseFinishDrag(button == MouseButton::Left, mouseState.mouseDownPos, mouseState.pos);
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
	for (auto n : objectTree->selectedItems)
		center += n->transform->getGlobalPosition() / (float)objectTree->selectedItems.size();
	return center;
}

void TienEdit::finishCurrentTransformAction()
{
	if (activeTool == EditTool::TRANSLATE)
	{
		glm::vec3 diff = originalPosition - getSelectionCenter();
		std::vector<Action*> group;
		for (auto n : objectTree->selectedItems)
			group.push_back(new NodeMoveAction(n, n->transform->getGlobalPosition() + diff, n->transform->getGlobalPosition()));
		actions.push_back(new GroupAction(group));
		activeTool = EditTool::NONE;
		cacheSelection = true;
		updateComponentsPanel(); //TODO: don't make it update all elements, but just the proper textboxes
	}
	else if (activeTool == EditTool::TRANSLATEWITHOUTCHILDREN)
	{
		glm::vec3 diff = originalPosition - getSelectionCenter();
		std::vector<Action*> group;
		for (auto n : objectTree->selectedItems)
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

vrlib::tien::Component * TienEdit::loadCallback(const json & value, const json &completeFile)
{
	std::string type = value["type"].get<std::string>();
	if (componentFactory.find(type) != componentFactory.end())
		return componentFactory[type].second(value, completeFile);
	type[0] = ::toupper(type[0]);
	if (componentFactory.find(type) != componentFactory.end())
		return componentFactory[type].second(value, completeFile);
	return nullptr;
}



class ComponentRenderProps
{
public:
	bool folded = false;
};

std::map<vrlib::tien::Component*, ComponentRenderProps> renderProps;

void TienEdit::updateComponentsPanel()
{
	menuOverlay.focussedComponent = nullptr;
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

	editorBuilder->beginGroup("Prefab");
	editorBuilder->addPrefabBox(node->prefabFile, [node](const std::string &newValue) { node->prefabFile = newValue; });
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
				menuOverlay.focussedComponent = nullptr;
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
				menuOverlay.focussedComponent = nullptr;
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



void TienEdit::rebakeSelectedLights()
{
	for (auto n : objectTree->selectedItems)
	{
		n->fortree([this, &n](const vrlib::tien::Node* nn)
		{
			if (nn->light && nn->light->baking == vrlib::tien::components::Light::Baking::baked)
				nn->light->rebake();
		});
	}
}

TienEdit::DelayedTask TienEdit::runDelayed(float time, std::function<void()> callback)
{
	static DelayedTask taskId = 1;
	DelayedTask newTask = taskId++;
	delayedTasks[newTask] = std::make_tuple(time, callback);
	return newTask;
}

void TienEdit::stopDelayed(DelayedTask toStop)
{
	delayedTasks.erase(delayedTasks.find(toStop));
}
