#pragma once

#include <VrLib/tien/Component.h>

class StubComponent : public vrlib::tien::Component
{
public:
	json stub;
	json values;

	StubComponent();
	StubComponent(json v);

	virtual json toJson(json & meshes) const override;
	virtual void buildEditor(vrlib::tien::EditorBuilder* builder, bool folded);

};