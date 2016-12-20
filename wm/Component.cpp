#include "Component.h"
#include <VrLib/Kernel.h>

#include <GL/glew.h>
#include <glm/glm.hpp>

std::list<int*> Component::scissorStack;

void Component::scissorPush(int x, int y, int width, int height) 
{
	int windowHeight = vrlib::Kernel::getInstance()->getWindowHeight();
	int scissorEnabled;
	glGetIntegerv(GL_SCISSOR_TEST, &scissorEnabled);
	if (scissorEnabled)
	{
		int* scissor = new int[4];
		glGetIntegerv(GL_SCISSOR_BOX, scissor);
		scissorStack.push_back(scissor);

		x = glm::max(x, scissor[0]);
		y = glm::max(y, windowHeight - scissor[1] - scissor[3]);

	}
	glEnable(GL_SCISSOR_TEST);
	glScissor(x, windowHeight - y - height, width, height);
}


void Component::scissorPop()
{
	if (scissorStack.empty())
	{
		glDisable(GL_SCISSOR_TEST);
		return;
	}
	int* scissor = scissorStack.back();
	glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
	delete scissor;
	scissorStack.pop_back();
}