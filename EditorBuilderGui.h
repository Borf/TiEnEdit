#pragma once

#include <VrLib/tien/Component.h>

class Panel;
class Component;
class TienEdit;

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

	virtual void addTitle(const std::string & name) override;
	virtual TextBox* addTextBox(const std::string & value, std::function<void(const std::string&)> onChange) override;
	virtual void addCheckbox(bool value, std::function<void(bool)> onChange) override;
	virtual void addButton(const std::string &value, std::function<void()> onClick) override;
	virtual void addComboBox(const std::string &value, const std::vector<std::string> &values, std::function<void(const std::string&)> onClick) override;
	virtual void addBrowseButton(BrowseType type, std::function<void(const std::string &)> onClick) override;

	virtual void beginGroup(const std::string & name, bool verticalGroup = true) override;
	virtual void endGroup() override;

};