#include "Divider.h"
#include "../menu/MenuOverlay.h"

#include <glm/gtc/matrix_transform.hpp>
#include <VrLib/gl/Vertex.h>
#include <VrLib/Font.h>

Divider::Divider(const glm::ivec2 & position)
{
	this->position = position;
}

void Divider::draw(MenuOverlay* overlay)
{
	overlay->drawRect(glm::vec2(10, 487), glm::vec2(10 + 7, 487 + 1), absPosition, absPosition + size); //menubar
	overlay->flushVerts();

}
