#pragma once

#include <VrLib/tien/Component.h>
#include "wm/TextField.h"
class Panel;
class Component;
class TienEdit;

class ModelTextField : public TextField
{
public:
	ModelTextField(const std::string &value, const glm::ivec2 &pos);
	virtual void handleDrag(DragProperties* draggedObject) override;
};

class TextureTextField : public TextField
{
public:
	TextureTextField(const std::string &value, const glm::ivec2 &pos);
	virtual void handleDrag(DragProperties* draggedObject) override;
};


class GuiEditor : public vrlib::tien::EditorBuilder
{
public:
	Panel* panel;
	TienEdit* editor;

	float line = 0;
	std::vector<Component*> group;
	bool vertical;


	GuiEditor(TienEdit* editor, Panel* panel);

	virtual void reset();

	virtual TextComponent* addTitle(const std::string & name) override;
	virtual TextComponent* addTextBox(const std::string & value, std::function<void(const std::string&)> onChange) override;
	virtual TextComponent* addTextureBox(const std::string & value, std::function<void(const std::string&)> onChange) override;
	virtual TextComponent* addModelBox(const std::string & value, std::function<void(const std::string&)> onChange) override;
	virtual TextComponent* addLabel(const std::string & value) override;
	virtual ColorComponent * addColorBox(const glm::vec4 & value, std::function<void(const glm::vec4&)> onChange) override;
	virtual FloatComponent * addFloatBox(float value, float min, float max, std::function<void(float&)> onChange) override;
	virtual void addCheckbox(bool value, std::function<void(bool)> onChange) override;
	virtual void addButton(const std::string &value, std::function<void()> onClick) override;
	virtual void addSmallButton(const std::string &value, std::function<void()> onClick) override;
	virtual TextComponent* addComboBox(const std::string &value, const std::vector<std::string> &values, std::function<void(const std::string&)> onClick) override;

	virtual void beginGroup(const std::string & name, bool verticalGroup = true) override;
	virtual void endGroup() override;

	virtual void addDivider() override;
	virtual void updateComponentsPanel() override;

};