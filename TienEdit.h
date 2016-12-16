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


	glm::quat cameraRot;
	glm::quat cameraRotTo;
	glm::vec3 cameraPos;

	Component* focussedComponent;
	

	Component* renderPanel;
	SplitPanel* mainPanel;
	Tree<vrlib::tien::Node*>* objectTree;
	Panel* modelBrowsePanel;
	Panel* propertiesPanel;
	GuiEditor* editorBuilder;

	vrlib::math::Ray ray;


	enum class EditTool
	{
		NONE,
		TRANSLATE,
		SCALE,
		ROTATE
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


	void showBrowsePanel();
	void buildBrowsePanel(const std::string &directory);
	std::function<void(const std::string &)> browseCallback;

	MouseState mouseState;
	MouseState lastMouseState;

	std::vector<vrlib::tien::Node*> selectedNodes;
	bool cacheSelection = 0;
	GLuint selectionCache = 0;
	void updateComponentsPanel();

	std::list<Action*> actions;
	std::list<Action*> redoactions;
	void perform(Action* action);
	glm::vec3 getSelectionCenter() const;

	
	//file
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
};
