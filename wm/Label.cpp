#include "Label.h"
#include "../menu/MenuOverlay.h"

#include <glm/gtc/matrix_transform.hpp>
#include <VrLib/gl/Vertex.h>
#include <VrLib/Font.h>

Label::Label(const std::string & text, const glm::ivec2 & position)
{
	this->text = text;
	this->position = position;
}

void Label::draw(MenuOverlay* overlay)
{
	overlay->shader->setUniform(MenuOverlay::Uniforms::colorMult, glm::vec4(1, 1, 1, 1));
	overlay->shader->setUniform(MenuOverlay::Uniforms::modelMatrix, glm::scale(glm::translate(glm::mat4(), glm::vec3(absPosition + glm::ivec2(2, 12), 0)), glm::vec3(scale, scale, 1)));
	overlay->font->drawText<vrlib::gl::VertexP2T2>(text);

	overlay->shader->setUniform(MenuOverlay::Uniforms::modelMatrix, glm::mat4());
	overlay->shader->setUniform(MenuOverlay::Uniforms::colorMult, glm::vec4(1, 1, 1, 1));




}
