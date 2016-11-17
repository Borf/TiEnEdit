#pragma once

#include <vrlib/NormalApp.h>
#include <vrlib/tien/Tien.h>
#include <glm/glm.hpp>
#include <vector>


class TienEdit : public vrlib:: NormalApp
{
	vrlib::tien::Tien tien;
public:


	TienEdit(const std::string &filename);
	~TienEdit();

	virtual void init() override;
	virtual void preFrame(double frameTime, double totalTime) override;

	virtual void draw() override;

	virtual void mouseMove(int x, int y) override;
	virtual void mouseScroll(int offset) override;




};
