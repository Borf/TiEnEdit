#define _CRT_SECURE_NO_WARNINGS
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

#include <VrLib/util.h>
#include <glm/glm.hpp>
#include <sstream>

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


inline GuiEditor::TextComponent* GuiEditor::addPrefabBox(const std::string & value, std::function<void(const std::string&)> onChange)
{
	TextField* field = new PrefabTextField(value, glm::ivec2(100, line));
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

GuiEditor::ColorComponent * GuiEditor::addColorBox(const glm::vec4 & value, std::function<void(const glm::vec4&)> onChange)
{
	class ColorBox : public TextField, public ColorComponent
	{
	public:
		glm::vec4 color;

		std::function<void(const glm::vec4 &newColor)> oncolorChange;

		ColorBox(const glm::vec4 &color, const glm::ivec2 &pos) : TextField("", pos)
		{
			this->color = color;
			char rgb[10];
			sprintf(rgb, "%02X%02X%02X", (int)(color.r * 255), (int)(color.g * 255), (int)(color.b * 255));
			this->value = rgb;
			this->onChange = [this]()
			{
				std::stringstream c(value);
				unsigned int rgb;
				c >> std::hex >> rgb;
				this->color.b = ((rgb >> 0) & 255) / 255.0f;
				this->color.g = ((rgb >> 8) & 255) / 255.0f;
				this->color.r = ((rgb >> 16) & 255) / 255.0f;
				oncolorChange(this->color);
			};
		}


		virtual void draw(MenuOverlay* overlay) override
		{
			TextField::draw(overlay);

			overlay->shader->setUniform(MenuOverlay::Uniforms::colorMult, color);
			overlay->flushVerts();
			overlay->drawRect(glm::vec2(58, 495), glm::vec2(58 + 7, 495 + 7), absPosition + glm::ivec2(size.x - 20, 2), absPosition + glm::ivec2(size.x-4, size.y-2)); //val
			overlay->flushVerts();
			overlay->shader->setUniform(MenuOverlay::Uniforms::colorMult, glm::vec4(1, 1, 1, 1));




			if (focussed)
			{
				overlay->drawRect(glm::vec2(128, 328), glm::vec2(128 + 37, 328 + 33), absPosition + glm::ivec2(0, size.y), absPosition + glm::ivec2(size.x, size.y + 350 + 5)); //dropdown
				overlay->drawRect(glm::vec2(512, 0), glm::vec2(512+200, 200), absPosition + glm::ivec2(0, size.y)); //dropdown

				glm::vec3 hsv = vrlib::util::rgb2hsv(glm::vec3(color));

				if (glm::isinf(hsv.x) || glm::isnan(hsv.x))
					hsv.x = 0;

				float rad = 90 * hsv.y;
				float a = glm::radians(hsv.x) - glm::half_pi<float>();
				overlay->drawRect(glm::vec2(114, 487), glm::vec2(114 + 7, 487+7), absPosition + glm::ivec2(0, size.y) + glm::ivec2(100-3 + rad * cos(a),100-3 + rad * sin(a))); //dropdown
				
				overlay->drawRect(glm::vec2(42, 487), glm::vec2(42 + 7, 487 + 7), absPosition + glm::ivec2(5, size.y + 200 + 22), absPosition + glm::ivec2(size.x - 10, size.y + 200 + 25)); //hue
				overlay->drawRect(glm::vec2(42, 487), glm::vec2(42 + 7, 487 + 7), absPosition + glm::ivec2(5, size.y + 200 + 62), absPosition + glm::ivec2(size.x - 10, size.y + 200 + 65)); //sat
				overlay->drawRect(glm::vec2(42, 487), glm::vec2(42 + 7, 487 + 7), absPosition + glm::ivec2(5, size.y + 200 + 102), absPosition + glm::ivec2(size.x - 10, size.y + 200 + 105)); //val

				overlay->drawRect(glm::vec2(224, 480), glm::vec2(224 + 8, 480 + 18), absPosition + glm::ivec2(5 + 185 * hsv.x / 360.0f - 2, size.y + 200 + 15)); //hue
				overlay->drawRect(glm::vec2(224, 480), glm::vec2(224 + 8, 480 + 18), absPosition + glm::ivec2(5 + 185 * hsv.y - 2, size.y + 200 + 55)); //sat
				overlay->drawRect(glm::vec2(224, 480), glm::vec2(224 + 8, 480 + 18), absPosition + glm::ivec2(5 + 185 * hsv.z - 2, size.y + 200 + 95)); //val
				overlay->flushVerts();

				overlay->drawText("Hue", absPosition + glm::ivec2(5, size.y + 200 + 10));
				overlay->drawText("Saturation", absPosition + glm::ivec2(5, size.y + 200 + 10 + 40));
				overlay->drawText("Value", absPosition + glm::ivec2(5, size.y + 200 + 10 + 80));

			}
		}

		virtual bool mouseDrag(bool leftButton, const glm::ivec2 &startPos, const glm::ivec2 &mousePos, const glm::ivec2 & lastMousePos)
		{
			glm::ivec2 rel = mousePos - absPosition - glm::ivec2(100, 100 + size.y);

			float dist = glm::length(glm::vec2(rel)) / 90.0f;
			float angle = glm::atan((float)rel.y, (float)rel.x);

			float v = vrlib::util::rgb2hsv(glm::vec3(color)).z;

			if (dist < 1)
			{
				color = glm::vec4(vrlib::util::hsv2rgb(glm::vec3(glm::degrees(angle)+90, dist, v)), 1);
				char rgb[10];
				sprintf(rgb, "%02X%02X%02X", (int)(color.r * 255), (int)(color.g * 255), (int)(color.b * 255));
				this->value = rgb;
				if(oncolorChange)
					oncolorChange(this->color);
			}


			if (mousePos.y > absPosition.y + size.y + 200 + 10 &&
				mousePos.y < absPosition.y + size.y + 200 + 40)
			{
				glm::vec3 hsv = vrlib::util::rgb2hsv(glm::vec3(color));
				hsv.x = glm::min(1.0f, (mousePos.x - absPosition.x - 5) / 185.0f) * 360;
				color = glm::vec4(vrlib::util::hsv2rgb(hsv), 1);
				char rgb[10];
				sprintf(rgb, "%02X%02X%02X", (int)(color.r * 255), (int)(color.g * 255), (int)(color.b * 255));
				this->value = rgb;
				if (oncolorChange)
					oncolorChange(this->color);
			}
			if (mousePos.y > absPosition.y + size.y + 200 + 50 &&
				mousePos.y < absPosition.y + size.y + 200 + 80)
			{
				glm::vec3 hsv = vrlib::util::rgb2hsv(glm::vec3(color));
				hsv.y = glm::clamp((mousePos.x - absPosition.x - 5) / 185.0f, 0.0f, 1.0f);
				color = glm::vec4(vrlib::util::hsv2rgb(hsv), 1);
				char rgb[10];
				sprintf(rgb, "%02X%02X%02X", (int)(color.r * 255), (int)(color.g * 255), (int)(color.b * 255));
				this->value = rgb;
				if (oncolorChange)
					oncolorChange(this->color);
			}
			if (mousePos.y > absPosition.y + size.y + 200 + 90 &&
				mousePos.y < absPosition.y + size.y + 200 + 120)
			{
				glm::vec3 hsv = vrlib::util::rgb2hsv(glm::vec3(color));
				hsv.z = glm::clamp((mousePos.x - absPosition.x - 5) / 185.0f, 0.0f, 1.0f);
				color = glm::vec4(vrlib::util::hsv2rgb(hsv), 1);
				char rgb[10];
				sprintf(rgb, "%02X%02X%02X", (int)(color.r * 255), (int)(color.g * 255), (int)(color.b * 255));
				this->value = rgb;
				if (oncolorChange)
					oncolorChange(this->color);
			}

			
			return true;
		}

		virtual bool inComponent(const glm::ivec2 & pos) override
		{
			if (!focussed)
				return pos.x > absPosition.x && pos.x < absPosition.x + size.x && pos.y > absPosition.y && pos.y < absPosition.y + size.y;
			else
				return pos.x > absPosition.x && pos.x < absPosition.x + size.x && pos.y > absPosition.y && pos.y < absPosition.y + size.y + 350;
		}
		glm::vec4 getColor() const override { return color; }
		void setColor(const glm::vec4 &color) override { this->color = color; }
	};

	ColorBox* field = new ColorBox(value, glm::ivec2(100, line));
	field->size.x = 200;
	field->size.y = 20;
	field->oncolorChange = [onChange, field](const glm::vec4 &color)
	{
		if (onChange)
			onChange(field->color);
	};
	group.push_back(field);
	return field;
}

GuiEditor::FloatComponent * GuiEditor::addFloatBox(float value, float min, float max, std::function<void(float)> onChange)
{
	class FloatField : public TextField, public FloatComponent
	{
	public:
		float min;
		float max;

		FloatField(float value, const glm::ivec2 &pos) : TextField("", pos)
		{
			setValue(value);
		}

		virtual bool mouseDrag(bool leftButton, const glm::ivec2 &startPos, const glm::ivec2 &mousePos, const glm::ivec2 & lastMousePos)
		{
			if (!leftButton)
			{
				setValue(getValue() + (mousePos.x - lastMousePos.x) / 10.0f);
				onChange();
			}
			return true;
		}

		virtual bool keyChar(char character) override
		{
			std::string oldValue = value;
			float oldValueFloat = getValue();
			int oldCursor = cursor;
			int oldSelectionEnd = selectionEnd;
			bool ret = TextField::keyChar(character);
			//check if the last key pressed actually contributed something. Don't check if it's a . and there's no period yet, or when there's a backspace
			if (ret && getValue() == oldValueFloat && (character != '.' || value.find('.') != value.rfind('.')) && character != 8 && value != "0" && value != "-" && value != "-0")
			{
				value = oldValue;
				selectionEnd = oldSelectionEnd;
				cursor = oldCursor;
				onChange();
			}
			return ret;
		}

		float getValue() const override { return (float)atof(value.c_str()); }
		void setValue(float v) override 
		{ 
			if (v < min)
				v = min;
			if (v > max)
				v = max;

			std::ostringstream ss;
			ss << v;
			value = ss.str();
		}
	};

	FloatField* field = new FloatField(value, glm::ivec2(100, line));
	field->min = min;
	field->max = max;
	field->setValue(value);
	field->size.x = 200;
	field->size.y = 20;
	field->onChange = [onChange, field]()
	{
		onChange(field->getValue());
	};

	group.push_back(field);
	return field;
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

PrefabTextField::PrefabTextField(const std::string & value, const glm::ivec2 & pos) : TextField(value, pos)
{
	readonly = true;
}

void PrefabTextField::handleDrag(DragProperties * draggedObject)
{
	if (!draggedObject)
		return;
	if (draggedObject->type != DragProperties::Type::Prefab)
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

