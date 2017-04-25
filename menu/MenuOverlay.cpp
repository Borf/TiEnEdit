#include "MenuOverlay.h"

#include <VrLib/Texture.h>
#include <VrLib/Image.h>
#include <VrLib/json.hpp>
#include <VrLib/Font.h>

#include "Menu.h"
#include "MenuItem.h"
#include "SubMenuMenuItem.h"
#include "ToggleMenuItem.h"

#include <fstream>
#include <glm/gtc/matrix_transform.hpp>

inline static bool isInRectangle(const glm::vec2 &point, const glm::vec2 &a, const glm::vec2 &b) {
	return (a.x <= point.x && point.x <= b.x && a.y <= point.y && point.y <= b.y);
}


void MenuOverlay::init()
{
	font = new vrlib::TrueTypeFont("Tahoma", 16, 0);

	shader = new vrlib::gl::Shader<Uniforms>("data/TiEnEdit/shaders/overlay.vert", "data/TiEnEdit/shaders/overlay.frag");
	shader->bindAttributeLocation("a_position", 0);
	shader->bindAttributeLocation("a_texture", 1);
	shader->link();
	shader->registerUniform(Uniforms::modelMatrix, "matrix");
	shader->registerUniform(Uniforms::projectionMatrix, "projectionMatrix");
	shader->registerUniform(Uniforms::s_texture, "s_texture");
	shader->registerUniform(Uniforms::colorMult, "colorMult");
	shader->use();
	shader->setUniform(Uniforms::s_texture, 0);
	shader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
	skinTexture = vrlib::Texture::loadCached("data/TiEnEdit/textures/skin.png");

	menuOpen = -1;


}

void MenuOverlay::loadMenu(const std::string &menuFile)
{
	rootMenu = new Menu(json::parse(std::ifstream(menuFile)));
}


void MenuOverlay::drawInit()
{
	glDisable(GL_DEPTH_TEST);
	shader->use();
	shader->setUniform(Uniforms::projectionMatrix, glm::ortho(0.0f, (float)windowSize.x, (float)windowSize.y, 0.0f));
	shader->setUniform(Uniforms::modelMatrix, glm::mat4());
	shader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
	skinTexture->bind();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);



	verts.clear();
}


void MenuOverlay::drawPopups()
{
	for (auto m : popupMenus)
	{
		shader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
		shader->setUniform(Uniforms::modelMatrix, glm::mat4());
		skinTexture->bind();

		float width = 0;
		for (auto mm : m.second->menuItems)
			width = glm::max(width, font->textlen(mm->name) + 40);

		drawRect(glm::vec2(64, 416), glm::vec2(64 + 32, 416 + 32), m.first, m.first + glm::vec2(width, m.second->menuItems.size() * menuItemHeight + 10));
		flushVerts();

		float y = m.first.y + 5;
		for (auto mm : m.second->menuItems)
		{
			if (isInRectangle(mousePos, glm::vec2(m.first.x, y), glm::vec2(m.first.x + width, y + 11)))
			{
				shader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
				shader->setUniform(Uniforms::modelMatrix, glm::mat4());
				skinTexture->bind();
				drawRect(glm::vec2(96, 416), glm::vec2(128, 416 + 32), glm::vec2(m.first.x, y), glm::vec2(m.first.x + width, y + 15));
				flushVerts();
			}

			ToggleMenuItem* i = dynamic_cast<ToggleMenuItem*>(mm);
			if (i)
				drawText((i->getValue() ? "X " : "   ") + mm->name, glm::vec2(m.first.x + 5, y + 12), glm::vec4(1, 1, 1, 1), false);
			else
				drawText("   " + mm->name, glm::vec2(m.first.x + 5, y + 12), glm::vec4(1, 1, 1, 1), false);
			y += menuItemHeight;
		}

	}
	flushVerts();
}


bool MenuOverlay::click(bool button)
{
	//menubar
	if (mousePos.y < menuBarHeight)
	{
		if (menuOpen >= 0)
		{
			menuOpen = -1;
			popupMenus.clear();
			return lastClick = true;
		}
		else
		{
			float pos = 5;
			for (size_t i = 0; i < rootMenu->menuItems.size(); i++)
			{
				float width = font->textlen(rootMenu->menuItems[i]->name) + menuBarPadding;
				if (mousePos.x > pos && mousePos.x < pos + width)
					if (dynamic_cast<SubMenuMenuItem*>(rootMenu->menuItems[i]))
					{
						popupMenus.push_back(std::pair<glm::vec2, Menu*>(glm::vec2(pos, menuBarHeight), dynamic_cast<SubMenuMenuItem*>(rootMenu->menuItems[i])->menu));
						menuOpen = i;
					}
					else if (dynamic_cast<ActionMenuItem*>(rootMenu->menuItems[i]) && dynamic_cast<ActionMenuItem*>(rootMenu->menuItems[i])->callback)
						dynamic_cast<ActionMenuItem*>(rootMenu->menuItems[i])->callback();
				pos += width;
			}
		}
		return lastClick = true;
	}
	//toolbar
	else if (mousePos.y < menuBarHeight + toolBarHeight)
	{
		for (auto &button : toolbarButtons)
		{
			if (button.index >= 0)
			{
				if (mousePos.x > button.x && mousePos.x < button.x + 32)
				{
					button.callback();
					return lastClick = true;
				}
			}
		}
	}

	for (auto m : popupMenus)
	{
		float width = 0;
		for (auto mm : m.second->menuItems)
			width = glm::max(width, font->textlen(mm->name) + 40);

		if (isInRectangle(mousePos, m.first, m.first + glm::vec2(width, m.second->menuItems.size() * 12 + 10)))
		{
			int index = (int)((mousePos.y - m.first.y - 5) / menuItemHeight);
			if (index >= 0 && index < (int)m.second->menuItems.size())
			{
				if (dynamic_cast<ToggleMenuItem*>(m.second->menuItems[index]))
				{
					dynamic_cast<ToggleMenuItem*>(m.second->menuItems[index])->toggle();
				}

				if (dynamic_cast<ActionMenuItem*>(m.second->menuItems[index]))
				{
					if (dynamic_cast<ActionMenuItem*>(m.second->menuItems[index])->callback)
						dynamic_cast<ActionMenuItem*>(m.second->menuItems[index])->callback();
				}
				else if (dynamic_cast<SubMenuMenuItem*>(m.second->menuItems[index]))
				{

				}
			}
			menuOpen = -1;
			popupMenus.clear();
			return lastClick = true;
		}

	}

	popupMenus.clear();


	return lastClick = false;
}


bool MenuOverlay::wasClicked()
{
	if (mousePos.y < menuBarHeight)
		return true;

	for (auto m : popupMenus)
	{
		float width = 0;
		for (auto mm : m.second->menuItems)
			width = glm::max(width, font->textlen(mm->name) + 40);
		if (isInRectangle(mousePos, m.first, m.first + glm::vec2(width, m.second->menuItems.size() * 12 + 10)))
			return true;
	}
	return lastClick;
}

void MenuOverlay::hover()
{
	if (menuOpen >= 0 && mousePos.y < menuBarHeight)
	{
		float pos = 5;
		for (size_t i = 0; i < rootMenu->menuItems.size(); i++)
		{
			float width = font->textlen(rootMenu->menuItems[i]->name) + menuBarPadding;
			if (mousePos.x > pos && mousePos.x < pos + width)
				if (dynamic_cast<SubMenuMenuItem*>(rootMenu->menuItems[i]))
				{
					if (i != menuOpen)
					{
						popupMenus.clear();
						popupMenus.push_back(std::pair<glm::vec2, Menu*>(glm::vec2(pos, menuBarHeight), dynamic_cast<SubMenuMenuItem*>(rootMenu->menuItems[i])->menu));
						menuOpen = i;
					}
				}
			pos += width;
		}
	}
}

void MenuOverlay::addToolbarButton(int icon, const std::string & name, std::function<void()> callback)
{
	ToolbarButton button;
	button.index = icon;
	button.tooltip = name;
	button.callback = callback;
	if (toolbarButtons.empty())
		button.x = 5;
	else
		button.x = toolbarButtons[toolbarButtons.size() - 1].x + 32;
	toolbarButtons.push_back(button);
}




void MenuOverlay::drawRootMenu()
{
	shader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
	glm::vec2 windowSize(this->windowSize);
	drawRect(glm::vec2(64, 416), glm::vec2(64 + 32, 416 + 32), glm::vec2(0, 0), glm::vec2(windowSize.x, menuBarHeight)); //menubar
	drawRect(glm::vec2(64, 416), glm::vec2(64 + 32, 416 + 32), glm::vec2(0, menuBarHeight), glm::vec2(windowSize.x, menuBarHeight+toolBarHeight)); //toolbar
	flushVerts();

	//draw the top menu bar
	float pos = 5;
	for (size_t i = 0; i < rootMenu->menuItems.size(); i++)
	{
		float newPos = pos + font->textlen(rootMenu->menuItems[i]->name) + menuBarPadding;
		if (isInRectangle(mousePos, glm::vec2(pos, 0), glm::vec2(newPos, 25)) || menuOpen == i)
		{
			drawRect(glm::vec2(96, 416), glm::vec2(128, 416 + 32), glm::vec2(pos, 2), glm::vec2(newPos, 23)); //menubar
			flushVerts();
		}
		drawText(rootMenu->menuItems[i]->name, glm::vec2(pos + menuBarPadding/2, 5+12), glm::vec4(1, 1, 1, 1), false);
		pos = newPos;
	}


	//draw the toolbar buttons
	for (const auto &button : toolbarButtons)
	{
		int i = button.index;
		if (i < 0)
			continue;

		if(isInRectangle(mousePos, glm::vec2(button.x, menuBarHeight + 3), glm::vec2(button.x + 32, menuBarHeight + 32 + 3)))
			drawRect(glm::vec2(96, 416), glm::vec2(128, 416 + 32), glm::vec2(button.x, menuBarHeight + 3), glm::vec2(button.x + 32, menuBarHeight + 32 + 3));
		else
			drawRect(glm::vec2(0, 416), glm::vec2(32, 416 + 32), glm::vec2(button.x, menuBarHeight + 3), glm::vec2(button.x + 32, menuBarHeight + 32 + 3));
		drawRect(glm::vec2(512-33, 1+33*i), glm::vec2(512-1, 1+33*i+32), glm::vec2(button.x, menuBarHeight + 3));
	}
	flushVerts();
}







void MenuOverlay::flushVerts()
{
	if (verts.empty())
		return;
	skinTexture->bind();
	vrlib::gl::setAttributes<vrlib::gl::VertexP2T2>(&verts[0]);
	glDrawArrays(GL_QUADS, 0, verts.size());
	verts.clear();
}


void MenuOverlay::drawText(const std::string &text, const glm::vec2 position, const glm::vec4 color /*= glm::vec4(1, 1, 1, 1)*/, bool shadow) const
{
	if (shadow)
	{
		shader->setUniform(Uniforms::modelMatrix, glm::translate(glm::mat4(), glm::vec3(position + glm::vec2(1, 1), 0)));
		shader->setUniform(Uniforms::colorMult, glm::vec4(0, 0, 0, 1));
		font->drawText<vrlib::gl::VertexP2T2>(text);
	}

	shader->setUniform(Uniforms::colorMult, color);
	shader->setUniform(Uniforms::modelMatrix, glm::translate(glm::mat4(), glm::vec3(position, 0)));
	font->drawText<vrlib::gl::VertexP2T2>(text);

	shader->setUniform(Uniforms::modelMatrix, glm::mat4());
	shader->setUniform(Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
}

void MenuOverlay::drawRect(const glm::vec2 &_srcTl, const glm::vec2 &_srcBr, const glm::vec2 &dstTl)
{

	glm::vec2 srcTl = _srcTl / glm::vec2(skinTexture->image->width, skinTexture->image->height);
	glm::vec2 srcBr = _srcBr / glm::vec2(skinTexture->image->width, skinTexture->image->height);
	glm::vec2 srcSize = (srcBr - srcTl);
	glm::vec2 _srcSize = (_srcBr - _srcTl);

	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x,				dstTl.y),				glm::vec2(srcTl.x,				1-srcTl.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x,	dstTl.y),				glm::vec2(srcTl.x + srcSize.x,	1-srcTl.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x,	dstTl.y + _srcSize.y),	glm::vec2(srcTl.x + srcSize.x,	1-srcTl.y - srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x,				dstTl.y + _srcSize.y),	glm::vec2(srcTl.x,				1-srcTl.y - srcSize.y)));

}

void MenuOverlay::drawRect(const glm::vec2 &_srcTl, const glm::vec2 &_srcBr, const glm::vec2 &dstTl, const glm::vec2 &dstBr)
{
	glm::vec2 srcTl = _srcTl / glm::vec2(skinTexture->image->width, skinTexture->image->height);
	glm::vec2 srcBr = _srcBr / glm::vec2(skinTexture->image->width, skinTexture->image->height);
	glm::vec2 srcSize = (srcBr - srcTl) / 3.0f;
	glm::vec2 _srcSize = (_srcBr - _srcTl) / 3.0f;

	srcTl.y = 1 - srcTl.y;
	srcBr.y = 1 - srcBr.y;

	//top
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x, dstTl.y),								glm::vec2(srcTl.x,				srcTl.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x, dstTl.y),					glm::vec2(srcTl.x + srcSize.x,	srcTl.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x, dstTl.y + _srcSize.y),	glm::vec2(srcTl.x + srcSize.x,	srcTl.y - srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x, dstTl.y + _srcSize.y),					glm::vec2(srcTl.x,				srcTl.y - srcSize.y)));

	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x, dstTl.y),					glm::vec2(srcTl.x + srcSize.x,	srcTl.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x - _srcSize.x, dstTl.y),					glm::vec2(srcBr.x - srcSize.x,	srcTl.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x - _srcSize.x, dstTl.y + _srcSize.y),	glm::vec2(srcBr.x - srcSize.x,	srcTl.y - srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x, dstTl.y + _srcSize.y),	glm::vec2(srcTl.x + srcSize.x,	srcTl.y - srcSize.y)));

	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x - _srcSize.x, dstTl.y),					glm::vec2(srcBr.x - srcSize.x,	srcTl.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x, dstTl.y),								glm::vec2(srcBr.x,				srcTl.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x, dstTl.y + _srcSize.y),					glm::vec2(srcBr.x,				srcTl.y - srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x - _srcSize.x, dstTl.y + _srcSize.y),	glm::vec2(srcBr.x - srcSize.x,	srcTl.y - srcSize.y)));
	//middle
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x, dstTl.y + _srcSize.y),					glm::vec2(srcTl.x,				srcTl.y - srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x, dstTl.y + _srcSize.y),	glm::vec2(srcTl.x + srcSize.x,	srcTl.y - srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x, dstBr.y - _srcSize.y),	glm::vec2(srcTl.x + srcSize.x,	srcBr.y + srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x, dstBr.y - _srcSize.y),					glm::vec2(srcTl.x,				srcBr.y + srcSize.y)));

	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x, dstTl.y + _srcSize.y),	glm::vec2(srcTl.x + srcSize.x,	srcTl.y - srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x - _srcSize.x, dstTl.y + _srcSize.y),	glm::vec2(srcBr.x - srcSize.x,	srcTl.y - srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x - _srcSize.x, dstBr.y - _srcSize.y),	glm::vec2(srcBr.x - srcSize.x,	srcBr.y + srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x, dstBr.y - _srcSize.y),	glm::vec2(srcTl.x + srcSize.x,	srcBr.y + srcSize.y)));

	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x - _srcSize.x, dstTl.y + _srcSize.y),	glm::vec2(srcBr.x - srcSize.x,	srcTl.y - srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x, dstTl.y + _srcSize.y),					glm::vec2(srcBr.x,				srcTl.y - srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x, dstBr.y - _srcSize.y),					glm::vec2(srcBr.x,				srcBr.y + srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x - _srcSize.x, dstBr.y - _srcSize.y),	glm::vec2(srcBr.x - srcSize.x,	srcBr.y + srcSize.y)));
	//bottom
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x, dstBr.y - _srcSize.y),					glm::vec2(srcTl.x,				srcBr.y + srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x, dstBr.y - _srcSize.y),	glm::vec2(srcTl.x + srcSize.x,	srcBr.y + srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x, dstBr.y),					glm::vec2(srcTl.x + srcSize.x,	srcBr.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x, dstBr.y),								glm::vec2(srcTl.x,				srcBr.y)));

	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x, dstBr.y - _srcSize.y),	glm::vec2(srcTl.x + srcSize.x,	srcBr.y + srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x - _srcSize.x, dstBr.y - _srcSize.y),	glm::vec2(srcBr.x - srcSize.x,	srcBr.y + srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x - _srcSize.x, dstBr.y),					glm::vec2(srcBr.x - srcSize.x,	srcBr.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x, dstBr.y),					glm::vec2(srcTl.x + srcSize.x,	srcBr.y)));

	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x - _srcSize.x, dstBr.y - _srcSize.y),	glm::vec2(srcBr.x - srcSize.x,	srcBr.y + srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x, dstBr.y - _srcSize.y),					glm::vec2(srcBr.x,				srcBr.y + srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x, dstBr.y),								glm::vec2(srcBr.x,				srcBr.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstBr.x - _srcSize.x, dstBr.y),					glm::vec2(srcBr.x -	srcSize.x,	srcBr.y)));

}