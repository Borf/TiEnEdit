#pragma once

#include "Component.h"
#include <string>
#include <functional>

namespace vrlib
{
	class Texture;
}

class Image : public Component
{
protected:
	vrlib::Texture* texture;
	glm::vec2 _srcTl;
	glm::vec2 _srcBr;
public:

	std::function<void()> onClick;


	Image(vrlib::Texture* texture, const glm::ivec2& position, const glm::ivec2 &size, const glm::ivec2 &tl = glm::ivec2(0, 0), const glm::ivec2 &br = glm::ivec2(-1, -1));
	void draw(MenuOverlay* overlay) override;
	bool click(bool, const glm::ivec2 &, int clickCount) override;;
};