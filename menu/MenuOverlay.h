#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <VrLib/gl/shader.h>
#include <VrLib/gl/Vertex.h>

class Menu;
class Button;
class TextField;
class Panel;
class Label;
class SplitPanel;
class Component;

namespace vrlib
{
	class Texture;
	class TrueTypeFont;
}

class MenuOverlay
{
	//toolbar stuff
	class ToolbarButton
	{
	public:
		int x;
		int index;
		std::string tooltip;
		std::function<void()> callback;
	};
	std::vector<ToolbarButton> toolbarButtons;

public:
	MenuOverlay(const MenuOverlay&) = delete;				//do not copy
	MenuOverlay& operator =(const MenuOverlay&) = delete;	//do not copy
	MenuOverlay() {};
	
	
	//some settings
	const int menuBarHeight = 25;
	const int menuBarPadding = 40;
	const int menuItemHeight = 15;
	const int toolBarHeight = 36;

	// properties used for menu/overlay
	int menuOpen;
	std::vector<std::pair<glm::vec2, Menu*> > popupMenus;
	glm::ivec2 windowSize;
	glm::vec2 mousePos;
	bool lastClick;
	Menu* rootMenu;

	//panels
	Component* focussedComponent;
	SplitPanel* mainPanel;



	struct
	{
		bool shown = false;
		std::string title;
		std::string defaultValue;
		TextField* inputText;
		Label* lblDescription;
		Button* btnOk;
		Button* btnCancel;
		Panel* panel = nullptr;
		std::function<void(const std::string &)> callback;
	} inputDialog;


	// graphics
	enum class Uniforms
	{
		projectionMatrix,
		modelMatrix,
		s_texture,
		colorMult
	};
	vrlib::gl::Shader<Uniforms>* shader;
	vrlib::Texture* skinTexture;
	vrlib::TrueTypeFont* font;
	std::vector<vrlib::gl::VertexP2T2> verts;

	//main interaction, use these for windows and menus
	void loadMenu(const std::string &menuFile);

	void addToolbarButton(int icon, const std::string &name, std::function<void()> callback);
	void addToolbarSeperator() { addToolbarButton(-1, "", []() {}); }

	void showInputDialog(const std::string &title, const std::string defaultValue, std::function<void(const std::string &)> callback);

	//automatically called in tienedit
	inline void setWindowSize(const glm::ivec2 &size) { this->windowSize = size; }
	void init();
	void drawInit();
	void drawPopups();
	void drawRootMenu();

	bool click(bool button);
	bool wasClicked();
	void hover();
	bool mouseUp(bool button);
	bool mouseScroll(int offset);
	bool mouseMove(const glm::ivec2 &pos);

	Component* getRootComponent();


	//drawing helper functions
	void flushVerts();
	void drawText(const std::string &text, const glm::vec2 position, const glm::vec4 color = glm::vec4(1, 1, 1, 1), bool shadow = true) const;
	void drawRect(const glm::vec2 &srcTl, const glm::vec2 &srcBr, const glm::vec2 &dstTl, const glm::vec2 &dstBr);
	void drawRect(const glm::vec2 &srcTl, const glm::vec2 &srcBr, const glm::vec2 &dstTl);

};