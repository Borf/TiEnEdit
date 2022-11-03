#pragma once

#include <VrLib/tien/Component.h>

class StubComponent : public vrlib::tien::Component
{
public:
	nlohmann::json stub;
	nlohmann::json values;

	StubComponent();
	StubComponent(nlohmann::json v);

	virtual nlohmann::json toJson(nlohmann::json & meshes) const override;
	virtual void buildEditor(vrlib::tien::EditorBuilder* builder, bool folded);

};