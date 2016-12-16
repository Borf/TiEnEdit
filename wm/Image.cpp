#include "Image.h"
#include "../menu/MenuOverlay.h"

#include <glm/gtc/matrix_transform.hpp>
#include <VrLib/gl/Vertex.h>
#include <VrLib/Font.h>
#include <VrLib/Texture.h>
#include <VrLib/Image.h>

Image::Image(vrlib::Texture* texture, const glm::ivec2& position, const glm::ivec2 &size, const glm::ivec2 &tl, const glm::ivec2 &br)
{
	this->texture = texture;
	this->position = position;
	this->size = size;

	_srcTl = tl;
	_srcBr = br;
	if (br == glm::ivec2(-1, -1))
		_srcBr = glm::vec2(this->texture->image->width, this->texture->image->height);
}

void Image::draw(MenuOverlay* overlay)
{
	overlay->shader->setUniform(MenuOverlay::Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
	overlay->shader->setUniform(MenuOverlay::Uniforms::modelMatrix, glm::translate(glm::mat4(), glm::vec3(absPosition, 0)));
	std::vector<vrlib::gl::VertexP2T2> verts;

	glm::vec2 dstTl(0, 0);
	glm::vec2 dstBr(size);


	glm::vec2 srcTl = _srcTl / glm::vec2(texture->image->width, texture->image->height);
	glm::vec2 srcBr = _srcBr / glm::vec2(texture->image->width, texture->image->height);
	glm::vec2 srcSize = (srcBr - srcTl);
	glm::vec2 _srcSize = (_srcBr - _srcTl);

	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x,				dstTl.y),				glm::vec2(srcTl.x,				1-srcTl.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x,	dstTl.y),				glm::vec2(srcTl.x + srcSize.x,	1-srcTl.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x + _srcSize.x,	dstTl.y + _srcSize.y),	glm::vec2(srcTl.x + srcSize.x,	1-srcTl.y - srcSize.y)));
	verts.push_back(vrlib::gl::VertexP2T2(glm::vec2(dstTl.x,				dstTl.y + _srcSize.y),	glm::vec2(srcTl.x,				1-srcTl.y - srcSize.y)));

	texture->bind();
	vrlib::gl::setAttributes<vrlib::gl::VertexP2T2>(&verts[0]);
	glDrawArrays(GL_QUADS, 0, verts.size());

	overlay->shader->setUniform(MenuOverlay::Uniforms::modelMatrix, glm::mat4());
	overlay->shader->setUniform(MenuOverlay::Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
}

inline bool Image::click(bool, const glm::ivec2 &, int clickCount) {
	if (onClick) 
	{ 
		onClick(); 
		return true; 
	} 
	return false;
}
