#include "StubComponent.h"
#include <VrLib/Log.h>

using vrlib::logger;
using vrlib::Log;

StubComponent::StubComponent()
{

}
StubComponent::StubComponent(json v)
{
	values = v;
}

json StubComponent::toJson(json & meshes) const
{
	json ret = values;
	ret["type"] = stub["name"];
	return ret;
}

void StubComponent::buildEditor(vrlib::tien::EditorBuilder* builder, bool folded)
{
	builder->addTitle(stub["name"].get<std::string>() + " (stub)");
	if (folded)
		return;

	for (auto &property : stub["properties"])
	{
		if (values.find(property["name"]) == values.end())
			values[property["name"].get<std::string>()] = property["default"];

		builder->beginGroup(property["name"]);
		if (property["type"] == "string")
			builder->addTextBox(values[property["name"].get<std::string>()].get<std::string>(), [this, &property](const std::string &newValue) { values[property["name"].get<std::string>()] = newValue; });
		if (property["type"] == "model")
			builder->addModelBox(values[property["name"].get<std::string>()].get<std::string>(), [this, &property](const std::string &newValue) { values[property["name"].get<std::string>()] = newValue; });
		else if (property["type"] == "float")
			builder->addFloatBox(values[property["name"].get<std::string>()].get<float>(),
				property["min"].get<float>(),
				property["max"].get<float>(), [this, &property](float newValue) {values[property["name"].get<std::string>()] = newValue; });
		else if (property["type"] == "bool")
			builder->addCheckbox(values[property["name"].get<std::string>()].get<bool>(), [this, &property](bool newValue) {values[property["name"].get<std::string>()] = newValue; });
		else if (property["type"] == "color")
		{
			const json &colorValue = values[property["name"].get<std::string>()];
			builder->addColorBox(glm::vec4(colorValue[0], colorValue[1], colorValue[2], colorValue[3]), [this, &property](const glm::vec4 &newValue) {
				for (int i = 0; i < 4; i++)
					values[property["name"].get<std::string>()][i] = newValue[i];
			});
		}
		else if (property["type"] == "enum")
		{
			std::vector<std::string> enumValues;
			for (const std::string &v : property["values"])
				enumValues.push_back(v);
			builder->addComboBox(values[property["name"].get<std::string>()].get<std::string>(), enumValues, [this, &property](const std::string &newValue) {values[property["name"].get<std::string>()] = newValue; });
		}
		else
			vrlib::logger << "Unknown stub type: " << property["type"] << Log::newline;

		builder->endGroup();
	}


}