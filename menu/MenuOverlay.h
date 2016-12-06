#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <VrLib/gl/shader.h>
#include <VrLib/gl/Vertex.h>

class Menu;

namespace vrlib
{
	class Texture;
	class TrueTypeFont;
}

class MenuOverlay
{
public:
	MenuOverlay(const MenuOverlay&) = delete;				//do not copy
	MenuOverlay& operator =(const MenuOverlay&) = delete;	//do not copy
	MenuOverlay() {};
	
	
	const int menuBarHeight = 25;
	const int menuBarPadding = 40;
	const int menuItemHeight = 15;


	const int toolBarHeight = 36;

	
	int menuOpen;
	std::vector<std::pair<glm::vec2, Menu*> > popupMenus;
	glm::ivec2 windowSize;
	glm::vec2 mousePos;

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
	Menu* rootMenu;

	//main interaction, use these for windows and menus
	void loadMenu(const std::string &menuFile);


	inline void setWindowSize(const glm::ivec2 &size) { this->windowSize = size; }

	void init();
	void drawInit();
	void drawPopups();
	void drawRootMenu();


	bool click(bool button);
	void hover();



	void flushVerts();
	void drawText(const std::string &text, const glm::vec2 position, const glm::vec4 color = glm::vec4(1, 1, 1, 1), bool shadow = true) const;
	void drawRect(const glm::vec2 &srcTl, const glm::vec2 &srcBr, const glm::vec2 &dstTl, const glm::vec2 &dstBr);
	void drawRect(const glm::vec2 &srcTl, const glm::vec2 &srcBr, const glm::vec2 &dstTl);

};