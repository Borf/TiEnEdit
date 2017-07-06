#include "BrowsePanel.h"

#include "TienEdit.h"
#include "wm/Label.h"
#include "wm/Image.h"
#include "wm/SplitPanel.h"
#include "wm/ScrollPanel.h"
#include "wm/ComboBox.h"

#include <VrLib/util.h>
#include <VrLib/Texture.h>
#include <VrLib/Image.h>
#include <VrLib/Model.h>
#include <VrLib/gl/FBO.h>

#include <algorithm>
#include <filesystem> // windows only?
#include <glm/gtc/matrix_transform.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

BrowsePanel::BrowsePanel(TienEdit* editor)
{
	this->editor = editor;

	modelPreviewShader = new vrlib::gl::Shader<ModelPreviewUniforms>("data/TiEnEdit/shaders/modelpreview.vert", "data/TiEnEdit/shaders/modelpreview.frag");
	modelPreviewShader->bindAttributeLocation("a_position", 0);
	modelPreviewShader->bindAttributeLocation("a_normal", 1);
	modelPreviewShader->bindAttributeLocation("a_texcoord", 2);
	modelPreviewShader->link();
	modelPreviewShader->bindFragLocation("fragColor", 0);
	modelPreviewShader->registerUniform(ModelPreviewUniforms::projectionMatrix, "projectionMatrix");
	modelPreviewShader->registerUniform(ModelPreviewUniforms::modelMatrix, "modelMatrix");
	modelPreviewShader->registerUniform(ModelPreviewUniforms::viewMatrix, "viewMatrix");
	modelPreviewShader->registerUniform(ModelPreviewUniforms::s_texture, "s_texture");
	modelPreviewShader->registerUniform(ModelPreviewUniforms::textureFactor, "textureFactor");
	modelPreviewShader->registerUniform(ModelPreviewUniforms::color, "color");
	modelPreviewShader->use();
	modelPreviewShader->setUniform(ModelPreviewUniforms::s_texture, 0);



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
		this->size.y = 150 * glm::max(2, (int)glm::ceil((components.size() / 2) / (float)count));


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
	this->directory = directory;
	if (editor->browseToolbar.directory)
		editor->browseToolbar.directory->text = directory;

	for (auto c : components)
		delete c;
	components.clear();

	std::vector<std::string> files = vrlib::util::scandir(directory, editor->browseToolbar.searchFilter && editor->browseToolbar.searchFilter->value != "");

	files.erase(std::remove_if(files.begin(), files.end(), [this](const std::string &s)
	{

		if (s.size() == 0)
			return true;

		if (editor->browseToolbar.searchFilter && editor->browseToolbar.searchFilter->value != "")
		{
			std::vector<std::string> terms = vrlib::util::split(editor->browseToolbar.searchFilter->value, " ");
			bool match = true;
			for (auto &term : terms)
				if (s.find(term) == std::string::npos)
					match = false;
			if (!match)
				return true;
		}
		if (s[s.length() - 1] == '/')
			return false;

		FileType type = fileType(s);

		if (editor->browseToolbar.typeFilter && editor->browseToolbar.typeFilter->value != "all")
		{
			if (editor->browseToolbar.typeFilter->value == "models" && type != FileType::Model)
				return true;
			if (editor->browseToolbar.typeFilter->value == "textures" && type != FileType::Image && type != FileType::Video)
				return true;
			if (editor->browseToolbar.typeFilter->value == "prefabs" && type != FileType::Prefab)
				return true;
		}
		if (type != FileType::Other)
			return false;


		return true;
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
		Image* imgOverlay = nullptr;
		if (files[i][files[i].size() - 1] == '/')
		{ //folders
			if (std::tr2::sys::exists(directory + files[i] + ".icon.png"))
			{
				img = new Image(vrlib::Texture::loadCached(directory + files[i] + ".icon.png"), glm::ivec2(0, 0), glm::ivec2(120, 120));
				imgOverlay = new Image(vrlib::Texture::loadCached("data/tienedit/textures/folder.png"), glm::ivec2(0, 0), glm::ivec2(120, 120));
			}
			else
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
			FileType type = fileType(files[i]);
			if (type == FileType::Model)
			{
				std::string cacheFilename = "data/tienedit/cache/" + vrlib::util::replace(directory + files[i], "/", "_");
				struct stat result;
				if (stat((directory + files[i]).c_str(), &result) == 0)
					cacheFilename += "." + std::to_string(result.st_mtime) + ".png";

				if (!std::tr2::sys::exists(cacheFilename))
				{ //TODO: move this to a seperate method to generate thumbnail
					vrlib::gl::FBO* fbo = new vrlib::gl::FBO(128,128,true);
					
					auto model = vrlib::Model::getModel<vrlib::gl::VertexP3N3T2>(directory + files[i]);
					if (model)
					{
						glPushAttrib(GL_ALL_ATTRIB_BITS);
						fbo->bind();
						glViewport(0, 0, 128, 128);
						glClearColor(1, 1, 1, 0);
						glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
						

						glm::vec3 size = model->aabb.getSize();
						float farPlane = glm::max(size.x, glm::max(size.y, size.z));


						modelPreviewShader->use();
						modelPreviewShader->setUniform(ModelPreviewUniforms::projectionMatrix, glm::perspective(75.0f, 1.0f, 0.01f, farPlane * 10));
						glm::mat4 camera;
						camera = glm::scale(camera, glm::vec3(1, -1, 1));
						camera = glm::rotate(camera, glm::radians(-5.0f), glm::vec3(1, 0, 0));
						camera = glm::translate(camera, glm::vec3(0, 0, -farPlane * 5.0f));
						camera = glm::rotate(camera, glm::radians(40.0f), glm::vec3(1, 0, 0));

						modelPreviewShader->setUniform(ModelPreviewUniforms::viewMatrix, camera);
						modelPreviewShader->setUniform(ModelPreviewUniforms::textureFactor, 1.0f);
						modelPreviewShader->setUniform(ModelPreviewUniforms::color, glm::vec4(1,1,1,1));


						glEnable(GL_DEPTH_TEST);

						model->draw([this](const glm::mat4 &model)
						{
							modelPreviewShader->setUniform(ModelPreviewUniforms::modelMatrix, model);
						},
						[this](const vrlib::Material &material)
						{
							if (material.texture)
							{
								modelPreviewShader->setUniform(ModelPreviewUniforms::textureFactor, 1.0f);
								modelPreviewShader->setUniform(ModelPreviewUniforms::color, glm::vec4(1, 1, 1, 1));
								material.texture->bind();
							}
							else
							{
								modelPreviewShader->setUniform(ModelPreviewUniforms::textureFactor, 0.0f);
								modelPreviewShader->setUniform(ModelPreviewUniforms::color, material.color.diffuse);

							}
							return true;
						});

						glBindVertexArray(0);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
						glBindBuffer(GL_ARRAY_BUFFER, 0);

						glDisable(GL_DEPTH_TEST);
						glDisable(GL_CULL_FACE);
						glEnable(GL_BLEND);

						modelPreviewShader->setUniform(ModelPreviewUniforms::projectionMatrix, glm::ortho(0.0f, 128.0f, 128.0f, 0.0f, -100.0f, 100.0f));
						modelPreviewShader->setUniform(ModelPreviewUniforms::viewMatrix, glm::mat4());
						modelPreviewShader->setUniform(ModelPreviewUniforms::modelMatrix, glm::mat4());
						modelPreviewShader->setUniform(ModelPreviewUniforms::textureFactor, 1.0f);
						modelPreviewShader->setUniform(ModelPreviewUniforms::color, glm::vec4(1, 1, 1, 1));


						glm::vec2 tl(333, 128);
						glm::vec2 br(333 + 128, 128 + 128);
						tl /= glm::vec2(1024, 1024);
						br /= glm::vec2(1024, 1024);

						std::vector<vrlib::gl::VertexP3N3T2> verts;
						verts.push_back(vrlib::gl::VertexP3N3T2(glm::vec3(96,96,0), glm::vec3(0,0,1),		glm::vec2(tl.x, 1 - tl.y)));
						verts.push_back(vrlib::gl::VertexP3N3T2(glm::vec3(128,96,0), glm::vec3(0, 0, 1),	glm::vec2(br.x, 1 - tl.y)));
						verts.push_back(vrlib::gl::VertexP3N3T2(glm::vec3(128,128,0), glm::vec3(0, 0, 1),	glm::vec2(br.x, 1 - br.y)));
						verts.push_back(vrlib::gl::VertexP3N3T2(glm::vec3(96,128,0), glm::vec3(0, 0, 1),	glm::vec2(tl.x, 1 - br.y)));

						editor->menuOverlay.skinTexture->bind();
						vrlib::gl::setAttributes<vrlib::gl::VertexP3N3T2>(&verts[0]);
						glDrawArrays(GL_QUADS, 0, verts.size());
						


						fbo->unbind();
						editor->menuOverlay.shader->use();
						glPopAttrib();
						delete model;
						fbo->saveAsFile(cacheFilename);
						delete fbo;
					}


				}
				auto tex = vrlib::Texture::loadCached(cacheFilename);
				if(tex)
					img = new DraggableImage(editor, tex, glm::ivec2(0, 0), glm::ivec2(128, 128), glm::ivec2(0, 0), glm::ivec2(tex->image->width, tex->image->height), new DragProperties{ DragProperties::Type::Model, directory + files[i] });
				else
					img = new DraggableImage(editor, editor->menuOverlay.skinTexture, glm::ivec2(0, 0), glm::ivec2(128, 128), glm::ivec2(333, 128), glm::ivec2(333 + 128, 128 + 128), new DragProperties{ DragProperties::Type::Model, directory + files[i] });
			}
			else if (type == FileType::Prefab)
				img = new DraggableImage(editor, editor->menuOverlay.skinTexture, glm::ivec2(0, 0), glm::ivec2(128, 128), glm::ivec2(333, 256), glm::ivec2(333 + 128, 256 + 128), new DragProperties{ DragProperties::Type::Prefab, directory + files[i] });
			else if (type == FileType::Image || type == FileType::Video)
			{
				std::string cacheFilename = "data/tienedit/cache/" + vrlib::util::replace(directory + files[i], "/", "_");
				struct stat result;
				if (stat((directory + files[i]).c_str(), &result) == 0)
					cacheFilename += "." + std::to_string(result.st_mtime) + ".png";

				if (!std::tr2::sys::exists(cacheFilename))
				{
					vrlib::Image* thumb = new vrlib::Image(directory + files[i]);
					if (thumb && thumb->data && thumb->width > 0 && thumb->height > 0)
					{
						thumb->scale(128, 128);
						thumb->flipv();
						thumb->save(cacheFilename);
					}
					else
						cacheFilename = directory + files[i];
					if(thumb)
						delete thumb;
				}
				auto tex = vrlib::Texture::loadCached(cacheFilename);
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
		if (imgOverlay)
			components.push_back(imgOverlay);

		std::string name = files[i];
		if (name.rfind("/") != std::string::npos && name.rfind("/") != name.size()-1)
			name = name.substr(name.rfind("/") + 1);
		components.push_back(new Label(name, glm::ivec2(0, 0)));
	}
	if(editor->mainPanel->components.size() == 3) //TODO: ew
		dynamic_cast<ScrollPanel*>(dynamic_cast<SplitPanel*>(dynamic_cast<SplitPanel*>(editor->mainPanel->components[1])->components[1])->components[1])->scrollOffset = glm::ivec2(0, 0); //TODO: ew too
	onReposition(nullptr);
	editor->menuOverlay.focussedComponent = nullptr;
}

BrowsePanel::FileType BrowsePanel::fileType(const std::string & file)
{
	std::string extension = file;
	if (extension.find(".") != std::string::npos)
		extension = extension.substr(extension.rfind("."));
	std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);


	if (false ||
		extension == ".fbx" ||
		extension == ".obj" ||
		extension == ".ma" ||
		extension == ".lwo" ||
		extension == ".stl" ||
		extension == ".dae" ||
		extension == ".skn" ||
		extension == ".3ds" ||
		false)
		return FileType::Model;
	else if (false ||
		extension == ".png" ||
		extension == ".jpg" ||
		extension == ".jpeg" ||
		false)
		return FileType::Image;
	else if (false || 
		extension == ".mp4" ||
		extension == ".mkv" ||
		extension == ".avi" ||
		extension == ".mpg" ||
		extension == ".mpeg" ||
		false)
		return FileType::Image;
	else if (false ||
		extension == ".json" ||
		false)
		return FileType::Prefab;


	return FileType::Other;
}




DraggableImage::DraggableImage(TienEdit* editor, vrlib::Texture * texture, const glm::ivec2 & position, const glm::ivec2 & size, const glm::ivec2 & tl, const glm::ivec2 & br, DragProperties* dragProperties) : Image(texture, position, size, tl, br)
{
	this->editor = editor;
	this->dragProperties = dragProperties;
	this->focusable = true;
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
