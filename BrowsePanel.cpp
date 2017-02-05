#include "BrowsePanel.h"

#include "TienEdit.h"
#include "wm/Label.h"
#include "wm/Image.h"
#include "wm/SplitPanel.h"
#include "wm/ScrollPanel.h"

#include <VrLib/util.h>
#include <VrLib/Texture.h>
#include <VrLib/Image.h>
#include <VrLib/Model.h>

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

BrowsePanel::BrowsePanel(TienEdit* editor)
{
	this->editor = editor;
	rebuild("./data/models/testsphere/");
}


BrowsePanel::~BrowsePanel()
{
}

inline void BrowsePanel::onReposition(Component * parent)
{
	if(parent)
		this->size.x = parent->size.x;
	int count = glm::max(1, size.x / 128);
	int index = 0;
	if(parent && !components.empty())
		this->size.y = 150 * glm::max(2u, components.size() / 2 / count);


	for (size_t i = 0; i < components.size(); i++)
	{
		int x = (index % count) * 128 + 4;
		int y = (index / count) * 150 + 4;
		if (dynamic_cast<Image*>(components[i]))
		{
			components[i]->position.x = x;
			components[i]->position.y = y;
		}
		if (dynamic_cast<Label*>(components[i]))
		{
			components[i]->position.x = x;
			components[i]->position.y = y + 130;
			index++;
		}

	}
	Panel::onReposition(parent);
}



void BrowsePanel::rebuild(const std::string & directory)
{
	for (auto c : components)
		delete c;
	components.clear();

	std::vector<std::string> files = vrlib::util::scandir(directory);
	files.erase(std::remove_if(files.begin(), files.end(), [](const std::string &s)
	{
		if (s.size() == 0)
			return true;
		if (s[0] == '.')
			return true;
		if (s[s.length() - 1] == '/')
			return false;
		if (s.find("."))
		{
			std::string extension = s.substr(s.rfind("."));
			std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
			if (extension == ".fbx" || extension == ".obj" || extension == ".ma" || extension == ".lwo" || extension == ".stl" || extension == ".dae" ||
				extension == ".jpg" || extension == ".png" || extension == ".jpeg" || extension == ".mp4")
				return false;
			else
				return true;
		}
		return false;
	}), files.end());

	if (directory != "./")
		files.insert(files.begin(), "../");

	std::sort(files.begin(), files.end(), [](const std::string &a, const std::string &b)
	{
		bool isDirA = a[a.size() - 1] == '/';
		bool isDirB = b[b.size() - 1] == '/';
		if (isDirA && !isDirB)
			return true;
		else if (!isDirA && isDirB)
			return false;
		else
			return a < b;
	});


	for (size_t i = 0; i < files.size(); i++)
	{
		Image* img = nullptr;
		if (files[i][files[i].size() - 1] == '/')
		{
			img = new Image(editor->menuOverlay.skinTexture, glm::ivec2(0, 0), glm::ivec2(120, 120), glm::ivec2(333, 0), glm::ivec2(333 + 128, 128));
			img->onClick = [this, directory, i, files]()
			{
				std::string newDirectory = directory + files[i];
				if (files[i] == "../")
					newDirectory = directory.substr(0, directory.rfind("/", directory.size() - 2)) + "/";
				rebuild(newDirectory);
			};
		}
		else
		{
			std::string extension = files[i];
			if (extension.find(".") != std::string::npos)
				extension = extension.substr(extension.rfind("."));

			if(false ||
				extension == ".fbx" ||
				extension == ".obj" ||
				extension == ".ma" ||
				extension == ".lwo" ||
				extension == ".stl" ||
				extension == ".dae" ||
				extension == ".skn" ||
				false)
				img = new DraggableImage(editor, editor->menuOverlay.skinTexture, glm::ivec2(0, 0), glm::ivec2(128, 128), glm::ivec2(333, 128), glm::ivec2(333 + 128, 128 + 128), new DragProperties{DragProperties::Type::Model, directory + files[i] });
			else if (false ||
				extension == ".png" ||
				extension == ".jpg" ||
				extension == ".jpeg" ||
				extension == ".mp4" ||
				false)
			{
				auto tex = vrlib::Texture::loadCached(directory + files[i]);
				img = new DraggableImage(editor, tex, glm::ivec2(0, 0), glm::ivec2(128, 128), glm::ivec2(0, 0), glm::ivec2(tex->image->width, tex->image->height), new DragProperties{ DragProperties::Type::Texture, directory + files[i] });
			}
			else
				img = new Image(editor->menuOverlay.skinTexture, glm::ivec2(0, 0), glm::ivec2(128, 128), glm::ivec2(333, 0), glm::ivec2(333 + 128, 128));
			img->onClick = [this, directory, i, files]()
			{
				//browseCallback(directory + files[i]);
				//mainPanel->components[1] = renderPanel;
			};

		}
		if (img)
			components.push_back(img);
		components.push_back(new Label(files[i], glm::ivec2(0, 0)));
	}
	if(editor->mainPanel->components.size() == 3) //TODO: ew
		dynamic_cast<ScrollPanel*>(dynamic_cast<SplitPanel*>(editor->mainPanel->components[1])->components[1])->scrollOffset = glm::ivec2(0, 0); //TODO: ew too
	onReposition(nullptr);
	editor->focussedComponent = nullptr;
}




DraggableImage::DraggableImage(TienEdit* editor, vrlib::Texture * texture, const glm::ivec2 & position, const glm::ivec2 & size, const glm::ivec2 & tl, const glm::ivec2 & br, DragProperties* dragProperties) : Image(texture, position, size, tl, br)
{
	this->editor = editor;
	this->dragProperties = dragProperties;
	//TODO: simplify
}

bool DraggableImage::mouseDrag(bool leftButton, const glm::ivec2 & startPos, const glm::ivec2 & mousePos, const glm::ivec2 & lastMousePos)
{
	if (!editor->dragDrawCallback)
	{
		editor->dragDrawCallback = [this, startPos](const glm::ivec2 &mousePos)
		{
			glDisable(GL_SCISSOR_TEST);
			auto overlay = &editor->menuOverlay;

			overlay->shader->setUniform(MenuOverlay::Uniforms::colorMult, glm::vec4(1, 1, 1, 0.25f));
			overlay->shader->setUniform(MenuOverlay::Uniforms::modelMatrix, glm::translate(glm::mat4(), glm::vec3(absPosition- startPos, 0)));
			std::vector<vrlib::gl::VertexP2T2> verts;

			glm::vec2 dstTl(mousePos);
			glm::vec2 dstBr(size);


			glm::vec2 srcTl = _srcTl / glm::vec2(texture->image->width, texture->image->height);
			glm::vec2 srcBr = _srcBr / glm::vec2(texture->image->width, texture->image->height);
			glm::vec2 srcSize = (srcBr - srcTl);
			glm::vec2 _srcSize = (_srcBr - _srcTl);

			verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x, dstTl.y), glm::vec2(srcTl.x, 1 - srcTl.y)));
			verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + size.x, dstTl.y), glm::vec2(srcTl.x + srcSize.x, 1 - srcTl.y)));
			verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + size.x, dstTl.y + size.y), glm::vec2(srcTl.x + srcSize.x, 1 - srcTl.y - srcSize.y)));
			verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x, dstTl.y + size.y), glm::vec2(srcTl.x, 1 - srcTl.y - srcSize.y)));

			texture->bind();
			vrlib::gl::setAttributes<vrlib::gl::VertexP2T2>(&verts[0]);
			glDrawArrays(GL_QUADS, 0, verts.size());

			overlay->shader->setUniform(MenuOverlay::Uniforms::modelMatrix, glm::mat4());
			overlay->shader->setUniform(MenuOverlay::Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
		};
	}
	return true;
}

bool DraggableImage::mouseFinishDrag(bool leftButton, const glm::ivec2 & startPos, const glm::ivec2 & mousePos)
{
	editor->dragDrawCallback = nullptr;

	Component* c = editor->mainPanel->getComponentAt(mousePos);
	if (c)
		c->handleDrag(dragProperties);

	return false;
}
