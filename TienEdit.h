#pragma once

#include <vrlib/NormalApp.h>
#include <vrlib/tien/Tien.h>
#include <VrLib/math/Ray.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

#include "menu/MenuOverlay.h"
#include "wm/Tree.h"
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
	glm::vec3 cameraPos;

	Component* focussedComponent;
	
	GuiEditor* editorBuilder;
	Component* renderPanel;
	SplitPanel* mainPanel;
	Tree<vrlib::tien::Node*>* objectTree;
	Panel* modelBrowsePanel;
	Panel* propertiesPanel;

	vrlib::math::Ray ray;


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


	void updateComponentsPanel();
	std::vector<vrlib::tien::Node*> selectedNodes;
	bool cacheSelection = 0;
	GLuint selectionCache = 0;

	std::list<Action*> actions;
	void perform(Action* action);
	void undo();
	void redo();


	void save();
	void load();
};
