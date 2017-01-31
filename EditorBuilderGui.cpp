#include "EditorBuilderGui.h"

#include "wm/Component.h"
#include "wm/Label.h"
#include "wm/CheckBox.h"
#include "wm/ComboBox.h"
#include "wm/TextField.h"
#include "wm/Button.h"
#include "wm/Panel.h"
#include "wm/Divider.h"

#include "TienEdit.h"

#include <glm/glm.hpp>

GuiEditor::GuiEditor(TienEdit* editor, Panel * panel)
{
	this->panel = panel;
	this->editor = editor;
}

inline void GuiEditor::reset()
{
	line = 4;
}

inline GuiEditor::TextComponent* GuiEditor::addTitle(const std::string & name)
{
	Label* label = new Label(name, glm::ivec2(0, line));
	label->scale = 1.5f;
	label->size.y = 25;
	panel->components.push_back(label);
	line += 25;
	return label;
}

inline GuiEditor::TextComponent* GuiEditor::addTextBox(const std::string & value, std::function<void(const std::string&)> onChange)
{
	TextField* field = new TextField(value, glm::ivec2(100, line));
	field->size.x = 200;
	field->size.y = 20;
	field->onChange = [onChange, field]()
	{
		if (onChange)
			onChange(field->value);
	};
	group.push_back(field);
	return field;
}


inline GuiEditor::TextComponent* GuiEditor::addModelBox(const std::string & value, std::function<void(const std::string&)> onChange)
{
	TextField* field = new ModelTextField(value, glm::ivec2(100, line));
	field->size.x = 200;
	field->size.y = 20;
	field->onChange = [onChange, field]()
	{
		if (onChange)
			onChange(field->value);
	};
	group.push_back(field);
	return field;
}

inline GuiEditor::TextComponent* GuiEditor::addTextureBox(const std::string & value, std::function<void(const std::string&)> onChange)
{
	TextField* field = new TextureTextField(value, glm::ivec2(100, line));
	field->size.x = 200;
	field->size.y = 20;
	field->onChange = [onChange, field]()
	{
		if (onChange)
			onChange(field->value);
	};
	group.push_back(field);
	return field;
}
GuiEditor::TextComponent * GuiEditor::addLabel(const std::string & value)
{
	Label* field = new Label(value, glm::ivec2(100, line+2));
	field->size.x = 200;
	field->size.y = 20;
	group.push_back(field);
	return field;
}

inline void GuiEditor::addCheckbox(bool value, std::function<void(bool)> onChange)
{
	CheckBox* box = new CheckBox(value, glm::ivec2(100, line));
	box->onChange = [onChange, box]() { if (onChange) onChange(box->value); };
	group.push_back(box);
}

inline void GuiEditor::addButton(const std::string & value, std::function<void()> onClick)
{
	Button* button = new Button(value, glm::ivec2(100, line));
	button->size.x = 200;
	button->size.y = 20;
	button->onClick = onClick;
	group.push_back(button);

}

inline void GuiEditor::addSmallButton(const std::string & value, std::function<void()> onClick)
{
	Button* button = new Button(value, glm::ivec2(100, line));
	button->size.x = 20 + value.size() * 8;
	button->size.y = 20;
	button->onClick = onClick;
	group.push_back(button);

}


inline void GuiEditor::addDivider()
{
	Divider* c = new Divider(glm::ivec2(0, line + 3));
	c->size = glm::ivec2(300, 2);
	line += 20;

	panel->components.push_back(c);
}

inline GuiEditor::TextComponent* GuiEditor::addComboBox(const std::string & value, const std::vector<std::string>& values, std::function<void(const std::string&)> onClick)
{
	ComboBox* box = new ComboBox(value, glm::ivec2(100, line));
	box->values = values;
	box->size.x = 200;
	box->size.y = 20;
	box->onChange = [onClick, box]()
	{
		if (onClick)
			onClick(box->value);
	};
	group.push_back(box);
	return box;
}

void GuiEditor::addBrowseButton(BrowseType type, std::function<void(const std::string &)> onClick)
{
/*	Button* button = new Button("Browse", glm::ivec2(100, line));
	button->size.x = 200;
	button->size.y = 20;
	button->onClick = [this, onClick]()
	{
		editor->browseCallback = onClick;
		editor->showBrowsePanel();
	};
	group.push_back(button);*/
}

inline void GuiEditor::beginGroup(const std::string & name, bool verticalGroup)
{
	panel->components.push_back(new Label(name, glm::ivec2(0, line + 2)));
	vertical = verticalGroup;
}

inline void GuiEditor::endGroup()
{
	int i = 0;

	int widthRemaining = 200;
	int countRemaining = group.size();
	if (!vertical)
		for (auto c : group)
			if (c->size.x != 200)
			{
				widthRemaining -= c->size.x;
				countRemaining--;
			}
	int posX = 0;

	for (auto c : group)
	{
		if (vertical)
		{
			c->position.y = (int)line;
			line += glm::max(c->size.y, 20);
		}
		else
		{
			c->position.y = (int)line;
			c->position.x += posX;
			if(c->size.x == 200)
				c->size.x = widthRemaining / countRemaining;
			posX += c->size.x;
		}
		panel->components.push_back(c);
		i++;
	}



	if (!vertical)
		line += 20;
	line += 5;
	group.clear();
}

void GuiEditor::updateComponentsPanel()
{
	editor->updateComponentsPanel();
}

ModelTextField::ModelTextField(const std::string & value, const glm::ivec2 & pos) : TextField(value, pos)
{
	readonly = true;
}

void ModelTextField::handleDrag(DragProperties * draggedObject)
{
	if (!draggedObject)
		return;
	if (draggedObject->type != DragProperties::Type::Model)
		return;
	this->value = draggedObject->file;
	onChange();
}

TextureTextField::TextureTextField(const std::string & value, const glm::ivec2 & pos) : TextField(value, pos)
{
	readonly = true;
}

void TextureTextField::handleDrag(DragProperties * draggedObject)
{
	if (!draggedObject)
		return;
	if (draggedObject->type != DragProperties::Type::Texture)
		return;
	this->value = draggedObject->file;
	onChange();
}

