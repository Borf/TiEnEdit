#include "MenuOverlay.h"

#include <VrLib/Texture.h>
#include <VrLib/Image.h>
#include <VrLib/json.hpp>
#include <VrLib/Font.h>

#include "../TienEdit.h"
#include "Menu.h"
#include "MenuItem.h"
#include "SubMenuMenuItem.h"
#include "ToggleMenuItem.h"
#include "../wm/Panel.h"
#include "../wm/Button.h"
#include "../wm/TextField.h"
#include "../wm/Label.h"
#include "../wm/SplitPanel.h"

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

			SubMenuMenuItem* smi = dynamic_cast<SubMenuMenuItem*>(mm);
			ToggleMenuItem* i = dynamic_cast<ToggleMenuItem*>(mm);
			if (i)
				drawText((i->getValue() ? "X " : "   ") + mm->name, glm::vec2(m.first.x + 5, y + 12), glm::vec4(1, 1, 1, 1), false);
			else
				drawText("   " + mm->name, glm::vec2(m.first.x + 5, y + 12), glm::vec4(1, 1, 1, 1), false);
			if(smi)
				drawText("->", glm::vec2(m.first.x + width - 20, y + 12), glm::vec4(1, 1, 1, 1), false);
			y += menuItemHeight;
		}

	}

	if (inputDialog.shown)
	{
		inputDialog.panel->draw(this);
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

		if (isInRectangle(mousePos, m.first, m.first + glm::vec2(width, m.second->menuItems.size() * menuItemHeight + 10)))
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
					bool opened = false;
					auto newMenu = dynamic_cast<SubMenuMenuItem*>(m.second->menuItems[index])->menu;
					for(auto &m : popupMenus)
						if (m.second == newMenu)
							opened = true;
					if(!opened)
						popupMenus.push_back(std::pair<glm::vec2, Menu*>(m.first + glm::vec2(width, index*menuItemHeight), newMenu));
					return lastClick = true;
				}
			}
			menuOpen = -1;
			popupMenus.clear();
			return lastClick = true;
		}

	}

	popupMenus.clear();

	///

	Component* clickedComponent = getRootComponent()->getComponentAt(mousePos);

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
		focussedComponent->mouseDown(button, mousePos);


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
Component* MenuOverlay::getRootComponent()
{
	if (inputDialog.shown)
		return inputDialog.panel;
	return mainPanel;
}


bool MenuOverlay::mouseUp(bool button)
{
	Component* root = getRootComponent();


	Component* clickedComponent = root->getComponentAt(mousePos);

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


	if (root->click(button, mousePos, TienEdit::mouseState.clickCount))
		return true;
	if (root->mouseUp(button, mousePos))
		return true;

	return false;
}

bool MenuOverlay::mouseScroll(int offset)
{
	bool scrolled = false;
	Component* c;
	if ((c = getRootComponent()->getComponentAt(mousePos)))
	{
		scrolled |= c->scroll(offset / 10.0f);

		if (!scrolled)
			scrolled |= getRootComponent()->scrollRecursive(mousePos, offset / 10.0f);


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
	return scrolled;
}

bool MenuOverlay::mouseMove(const glm::ivec2 & pos)
{
	mousePos = pos;
	if (focussedComponent && focussedComponent->inComponent(TienEdit::mouseState.mouseDownPos) && (TienEdit::mouseState.left || TienEdit::mouseState.right))
	{
		bool dragged = focussedComponent->mouseDrag(TienEdit::mouseState.left, TienEdit::mouseState.mouseDownPos, TienEdit::mouseState.pos, TienEdit::lastMouseState.pos);
		if (!dragged)
		{

		}
		return dragged;
	}
	return false;
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

void MenuOverlay::showInputDialog(const std::string & title, const std::string defaultValue, std::function<void(const std::string&)> callback)
{
	inputDialog.shown = true;
	inputDialog.title = title;
	inputDialog.callback = callback;
	if (!inputDialog.panel)
	{

		inputDialog.panel = new Panel();
		inputDialog.panel->size = glm::vec2(500, 200);
		inputDialog.panel->absPosition = glm::ivec2(windowSize / 2 - inputDialog.panel->size / 2);

		inputDialog.btnOk = new Button("Ok", glm::ivec2(inputDialog.panel->absPosition + glm::ivec2(300, 170)));
		inputDialog.btnOk->size = glm::ivec2(200, 25);
		inputDialog.panel->components.push_back(inputDialog.btnOk);

		inputDialog.btnCancel = new Button("Cancel", glm::ivec2(inputDialog.panel->absPosition + glm::ivec2(10, 170)));
		inputDialog.btnCancel->size = glm::ivec2(200, 25);
		inputDialog.panel->components.push_back(inputDialog.btnCancel);

		inputDialog.inputText = new TextField(defaultValue, glm::ivec2(inputDialog.panel->absPosition + glm::ivec2(10, 100)));
		inputDialog.inputText->size = glm::ivec2(480, 25);
		inputDialog.panel->components.push_back(inputDialog.inputText);

		inputDialog.lblDescription = new Label(title, glm::ivec2(inputDialog.panel->absPosition + glm::ivec2(10, 10)));
		inputDialog.lblDescription->size = glm::ivec2(480, 25);
		inputDialog.panel->components.push_back(inputDialog.lblDescription);
	}
	else
	{
		inputDialog.inputText->setText(defaultValue);
		inputDialog.lblDescription->setText(title);
	}

	inputDialog.btnOk->onClick = [this, callback]() {inputDialog.shown = false; callback(inputDialog.inputText->getText());  };
	inputDialog.btnCancel->onClick = [this]() {inputDialog.shown = false; };


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