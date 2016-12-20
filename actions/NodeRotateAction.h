#pragma once

#include "Action.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

namespace vrlib
{
	namespace tien
	{
		class Node;
	}
}

class NodeRotateAction : public Action
{
public:
	vrlib::tien::Node* node;
	glm::quat originalRotation;
	glm::vec3 originalPosition;
	glm::quat newRotation;
	glm::vec3 newPosition;

	NodeRotateAction(vrlib::tien::Node* node);

	virtual void perform(TienEdit * editor) override;
	virtual void undo(TienEdit * editor) override;
};