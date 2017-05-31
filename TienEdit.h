#pragma once

#include <vrlib/NormalApp.h>
#include <vrlib/tien/Tien.h>
#include <VrLib/math/Ray.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

#include "wm/Tree.h"
#include "menu/MenuOverlay.h"
class Component;
class SplitPanel;
class Panel;
class Action;
class GuiEditor;
class GroupAction;
class BrowsePanel;

class ToggleMenuItem;

namespace vrlib
{
	class Kernel;
	namespace tien
	{
		class Node;
	}
}

struct MouseState
{
	glm::ivec2 pos;
	glm::ivec2 lastClickPos;
	glm::ivec2 mouseDownPos;
	int clickCount = 1;
	DWORD lastClickTime;
	union
	{
		struct
		{
			bool left;
			bool middle;
			bool right;
		};
		bool buttons[3] = { false, false, false };
	};
};

class TienEdit : public vrlib:: NormalApp
{
public:
	vrlib::Kernel* kernel;
	vrlib::tien::Tien tien;
	MenuOverlay menuOverlay;

	std::string fileName;


	enum class EditorShaderUniforms
	{
		projectionMatrix,
		modelViewMatrix,
		s_texture,
		textureFactor,
		color
	};
	vrlib::gl::Shader<EditorShaderUniforms>* shader;


	glm::quat cameraRot;
	glm::quat cameraRotTo;
	glm::vec3 cameraPos;


	vrlib::Model* ruler;

	Component* renderPanel;
	SplitPanel* mainPanel;
	Tree<vrlib::tien::Node*>* objectTree;
	BrowsePanel* browsePanel;
	Panel* propertiesPanel;
	GuiEditor* editorBuilder;

	vrlib::math::Ray ray;


	enum class EditTool
	{
		NONE,
		TRANSLATE,
		TRANSLATEWITHOUTCHILDREN,
		SCALE,
		ROTATE,
		ROTATELOCAL,
	} activeTool = EditTool::NONE;

	enum Axis
	{
		X = 1,
		Y = 2,
		Z = 4,
		XY = X | Y,
		XZ = X | Z,
		YZ = Y | Z,
		XYZ = X | Y | Z,
	} axis;

	glm::vec3 originalPosition;
	
	int editorScale;
	GroupAction* activeEditAction;

	float cameraSpeed = 0;


	TienEdit(const std::string &filename);
	~TienEdit();

	virtual void init() override;
	virtual void preFrame(double frameTime, double totalTime) override;

	virtual void draw() override;

	virtual void mouseMove(int x, int y) override;
	virtual void mouseScroll(int offset) override;
	virtual void mouseUp(MouseButton button) override;
	virtual void mouseDown(MouseButton button) override;
	virtual void keyDown(int button) override;
	virtual void keyUp(int button) override;
	virtual void keyChar(char character) override;

	void updateNodePointer(vrlib::tien::Node* oldNode, vrlib::tien::Node* newNode);
	bool checkNodePointer(vrlib::tien::Node* oldNode);


	//void showBrowsePanel();
	//void buildBrowsePanel(const std::string &directory);
	//std::function<void(const std::string &)> browseCallback;

	static MouseState mouseState;
	static MouseState lastMouseState;

	std::vector<vrlib::tien::Node*> selectedNodes;
	bool cacheSelection = 0;
	GLuint selectionCache = 0;
	void updateComponentsPanel();

	std::list<Action*> actions;
	std::list<Action*> redoactions;
	void perform(Action* action);
	glm::vec3 getSelectionCenter() const;

	void finishCurrentTransformAction();


	vrlib::tien::Component* loadCallback(const json &value, const json &completeFile);


	ToggleMenuItem* debugPhysics;
	ToggleMenuItem* debugCamera;


	std::function<void(const glm::ivec2 &mousePos)> dragDrawCallback = nullptr;

	
	//file
	void newScene();
	void save();
	void load();
	//edit
	void undo();
	void redo();
	void sortScene();

	//object
	void copy();
	void paste();
	void deleteSelection();
	void focusSelectedObject();
	void duplicate();

	//csg
	void csgUnion();

	void rebakeSelectedLights();
};
