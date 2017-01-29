#pragma once

#include "wm/Panel.h"
#include "wm/Image.h"

class TienEdit;
namespace vrlib { class Texture; }


class DraggableImage : public Image
{
	TienEdit* editor;
	DragProperties* dragProperties = nullptr;
public:
	DraggableImage(TienEdit* editor, vrlib::Texture* texture, const glm::ivec2& position, const glm::ivec2 &size, const glm::ivec2 &tl = glm::ivec2(0, 0), const glm::ivec2 &br = glm::ivec2(-1, -1), DragProperties* dragProperties = nullptr);
	virtual bool mouseDrag(bool leftButton, const glm::ivec2 &startPos, const glm::ivec2 &mousePos) override;
	virtual bool mouseFinishDrag(bool leftButton, const glm::ivec2 &startPos, const glm::ivec2 &mousePos) override;
};


class BrowsePanel : public Panel
{
	TienEdit* editor;
public:
	BrowsePanel(TienEdit* editor);
	~BrowsePanel();

	virtual void onReposition(Component* parent) override;
	virtual void rebuild(const std::string & directory);


};

