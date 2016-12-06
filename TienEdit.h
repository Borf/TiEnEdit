#pragma once

#include <vrlib/NormalApp.h>
#include <vrlib/tien/Tien.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

#include "menu/MenuOverlay.h"
class Component;

namespace vrlib
{
	class Kernel;
}

struct MouseState
{
	glm::ivec2 pos;
	union
	{
		struct
		{
			bool left;
			bool middle;
			bool right;
		};
		bool buttons[3] = { false, false, false };
	};
};


class TienEdit : public vrlib:: NormalApp
{
public:
	vrlib::Kernel* kernel;
	vrlib::tien::Tien tien;
	MenuOverlay menuOverlay;
	Component* panel;

	Component* renderPanel;

	glm::quat cameraRot;
	glm::vec3 cameraPos;



	TienEdit(const std::string &filename);
	~TienEdit();

	virtual void init() override;
	virtual void preFrame(double frameTime, double totalTime) override;

	virtual void draw() override;

	virtual void mouseMove(int x, int y) override;
	virtual void mouseScroll(int offset) override;
	virtual void mouseUp(MouseButton button) override;
	virtual void mouseDown(MouseButton button) override;


	MouseState mouseState;
	MouseState lastMouseState;
};
