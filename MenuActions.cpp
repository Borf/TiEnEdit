#define _CRT_SECURE_NO_WARNINGS
#include "TienEdit.h"

#include <direct.h>
#include <fstream>

#include <glm/gtc/matrix_transform.hpp>

#include "actions/Action.h"
#include "actions/NodeDeleteAction.h"
#include "actions/SelectionChangeAction.h"

#include <VrLib/Texture.h>
#include <VrLib/util.h>
#include <VrLib/csgjs.h>
#include <vrlib/tien/components/Camera.h>
#include <vrlib/tien/components/Transform.h>
#include <vrlib/tien/components/Light.h>
#include <vrlib/tien/components/StaticSkyBox.h>
#include <vrlib/tien/components/DynamicSkyBox.h>


void TienEdit::undo()
{
	if (actions.empty())
		return;
	Action* lastAction = actions.back();
	actions.pop_back();
	lastAction->undo(this);
	redoactions.push_back(lastAction);
}

void TienEdit::redo()
{
	if (redoactions.empty())
		return;
	Action* action = redoactions.back();
	redoactions.pop_back();
	action->perform(this);
	actions.push_back(action);
}


void TienEdit::newScene()
{
	int ret = MessageBox(nullptr, "Are you sure you want to make a new scene?", "Are you sure?", MB_YESNO | MB_ICONWARNING | MB_SYSTEMMODAL);
	if (ret == IDYES)
	{
		tien.scene.reset();
		{
			vrlib::tien::Node* n = new vrlib::tien::Node("Camera", &tien.scene);
			n->addComponent(new vrlib::tien::components::Transform(glm::vec3(0, 0, 0)));
			n->addComponent(new vrlib::tien::components::Camera());
			n->addComponent(new vrlib::tien::components::DynamicSkyBox());
		}
		objectTree->selectedItems.clear();
		objectTree->update();
	}
}


void TienEdit::save()
{
	char* filter = "All\0 * .*\0Scenes\0 * .json\0";
	char curdir[512];
	_getcwd(curdir, 512);

	std::string target = std::string(curdir) + "\\data\\virtueelpd\\scenes";

	HWND hWnd = GetActiveWindow();
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;

	char buf[256];
	ZeroMemory(buf, 256);
	strcpy(buf, "");
	ofn.lpstrFile = buf;
	ofn.nMaxFile = 256;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 2;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = target.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_ENABLESIZING;
	if (GetSaveFileName(&ofn))
	{
		_chdir(curdir);
		fileName = buf;
		std::replace(fileName.begin(), fileName.end(), '\\', '/');
		if (fileName.find(".") == std::string::npos)
			fileName += ".json";

		vrlib::logger << "Save" << vrlib::Log::newline;
		vrlib::logger << "Saving to " << fileName;
		json saveFile;
		saveFile["meshes"] = json::array();
		saveFile["scene"] = tien.scene.asJson(saveFile["meshes"]);
		std::ofstream(fileName) << std::setw(4) << saveFile;
	}
	_chdir(curdir);

}

void TienEdit::load()
{
	char* filter = "All\0 * .*\0Scenes\0 * .json\0";
	char curdir[512];
	_getcwd(curdir, 512);

	std::string target = std::string(curdir) + "\\data\\virtueelpd\\scenes";

	HWND hWnd = GetActiveWindow();
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;

	char buf[256];
	ZeroMemory(buf, 256);
	//strcpy(buf, fileName.c_str());
	ofn.lpstrFile = buf;
	ofn.nMaxFile = 256;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 0;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = target.c_str();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_ENABLESIZING;
	if (GetOpenFileName(&ofn))
	{
		_chdir(curdir);
		fileName = buf;
		std::replace(fileName.begin(), fileName.end(), '\\', '/');
		if (fileName.find(".") == std::string::npos)
			fileName += ".json";

		vrlib::logger << "Opening " << fileName << Log::newline;
		json saveFile = json::parse(std::ifstream(fileName));
		tien.scene.reset();
		tien.scene.fromJson(saveFile["scene"], saveFile, std::bind(&TienEdit::loadCallback, this, std::placeholders::_1, saveFile));

		//tien.scene.cameraNode = (vrlib::tien::Node*)tien.scene.findNodeWithComponent<vrlib::tien::components::Camera>(); //TODO: just for testing post processing filters
		objectTree->selectedItems.clear();
		objectTree->update();
	}
	_chdir(curdir);
}


void TienEdit::deleteSelection()
{
	vrlib::logger << "Delete" << vrlib::Log::newline;

	perform(new NodeDeleteAction(objectTree->selectedItems));
}




void toClipboard(const std::string &s) {
	OpenClipboard(nullptr);
	EmptyClipboard();
	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, s.size() + 1);
	if (!hg) {
		CloseClipboard();
		return;
	}
	memcpy(GlobalLock(hg), s.c_str(), s.size() + 1);
	GlobalUnlock(hg);
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();
	GlobalFree(hg);
}

std::string fromClipboard()
{
	OpenClipboard(nullptr);
	HANDLE hData = GetClipboardData(CF_TEXT);
	assert(hData);
	char* pszText = static_cast<char*>(GlobalLock(hData));
	assert(pszText);
	std::string text(pszText);
	GlobalUnlock(hData);
	CloseClipboard();
	return text;
}

void TienEdit::copy()
{
	vrlib::logger << "Copy" << vrlib::Log::newline;

	json clipboard;
	clipboard["nodes"] = json::array();
	for (auto c : objectTree->selectedItems)
	{
		// TODO: only copy parents, not children of selected parents
		clipboard["nodes"].push_back(c->asJson(clipboard["meshes"]));
	}

	std::string out = clipboard.dump();
	toClipboard(out);
}

void TienEdit::paste()
{
	vrlib::logger << "Paste" << vrlib::Log::newline;

	json clipboard = json::parse(std::stringstream(fromClipboard()));
	if (clipboard.is_null())
	{
		vrlib::logger << "Invalid json on clipboard" << vrlib::Log::newline;
		return;
	}

	objectTree->selectedItems.clear();
	for (const json &n : clipboard["nodes"])
	{
		vrlib::tien::Node* newNode = new vrlib::tien::Node("", &tien.scene);
		newNode->fromJson(n, clipboard);

		newNode->fortree([](vrlib::tien::Node* n) {
			n->guid = vrlib::util::getGuid();
		});

		objectTree->selectedItems.push_back(newNode);
	}
	objectTree->update();

	activeTool = EditTool::TRANSLATE;
	originalPosition = getSelectionCenter();
	axis = Axis::XYZ;

}





void TienEdit::focusSelectedObject()
{
	glm::vec3 lookat = getSelectionCenter();

	glm::mat4 mat = glm::lookAt(cameraPos, lookat, glm::vec3(0, 1, 0));
	cameraRotTo = glm::quat(mat);
}

void TienEdit::duplicate()
{
	json clipboard;
	clipboard["nodes"] = json::array();
	for (auto c : objectTree->selectedItems)
	{
		// TODO: only copy parents, not children of selected parents
		clipboard["nodes"].push_back(c->asJson(clipboard["meshes"]));
	}

	if (clipboard.is_null())
	{
		vrlib::logger << "Invalid json on clipboard" << vrlib::Log::newline;
		return;
	}

	objectTree->selectedItems.clear();
	for (const json &n : clipboard["nodes"])
	{
		vrlib::tien::Node* newNode = new vrlib::tien::Node("", &tien.scene);
		newNode->fromJson(n, clipboard);

		newNode->fortree([](vrlib::tien::Node* n) {
			n->guid = vrlib::util::getGuid();
		});

		objectTree->selectedItems.push_back(newNode);
	}
	objectTree->update();

	activeTool = EditTool::TRANSLATE;
	originalPosition = getSelectionCenter();
	axis = Axis::XYZ;
}



void TienEdit::newMesh(const std::string & name, vrlib::tien::components::MeshRenderer::Mesh * mesh)
{
	vrlib::tien::Node* n = new vrlib::tien::Node(name, &tien.scene);
	n->addComponent(new vrlib::tien::components::Transform());

	mesh->material.texture = vrlib::Texture::loadCached("data/tienedit/textures/stub.png");

	n->addComponent(new vrlib::tien::components::MeshRenderer(mesh));
	perform(new SelectionChangeAction(this, { n }));

}

void TienEdit::csgOperate(bool keepOld, CsgOp operation)
{
	csgjs_model total;

	for (auto n : objectTree->selectedItems)
	{
		auto m = n->getComponent<vrlib::tien::components::MeshRenderer>();
		if (!m)
			continue;

		glm::mat3 normalMatrix(glm::transpose(glm::inverse(glm::mat3(n->transform->globalTransform))));

		csgjs_model mesh;
		for (auto &m : m->mesh->vertices)
		{
			glm::vec3 pos(n->transform->globalTransform * glm::vec4(m.px, m.py, m.pz, 1));
			glm::vec3 normal(normalMatrix * glm::vec3(m.nx, m.ny, m.nz));
			mesh.vertices.push_back(csgjs_vertex{
				csgjs_vector{ pos.x, pos.y, pos.z },
				csgjs_vector{ normal.x, normal.y, normal.z },
				csgjs_vector{ m.tx, m.ty, 0 },
				csgjs_vector{ m.tanx, m.tany, m.tanz },
			});
		}

		for (auto &i : m->mesh->indices)
			mesh.indices.push_back(i);
		if (operation == CsgOp::Union)
			total = csgjs_union(total, mesh);
		else
		{
			if (n == objectTree->selectedItems[0]) // if this is the first item, don't subtract, but equal
				total = mesh;
			else if (operation == CsgOp::Difference)
				total = csgjs_difference(total, mesh);
			else if (operation == CsgOp::Intersect)
				total = csgjs_intersection(total, mesh);

		}
	}


	auto* newMesh = objectTree->selectedItems[0]->getComponent<vrlib::tien::components::MeshRenderer>()->mesh;
	newMesh->vertices.clear();
	newMesh->indices.clear();

	glm::mat4 inv = glm::inverse(objectTree->selectedItems[0]->transform->globalTransform);
	for (auto &m : total.vertices)
	{
		glm::vec3 pos(inv * glm::vec4(m.pos.x, m.pos.y, m.pos.z, 1));

		newMesh->vertices.push_back(vrlib::gl::VertexP3N2B2T2T2(
			glm::vec3(pos.x, pos.y, pos.z),
			glm::vec3(m.normal.x, m.normal.y, m.normal.z),
			glm::vec2(m.uv.x, m.uv.y),
			glm::vec3(m.tangent.x, m.tangent.y, m.tangent.z)
		));
	}
	for (auto &i : total.indices)
		newMesh->indices.push_back(i);
	objectTree->selectedItems[0]->getComponent<vrlib::tien::components::MeshRenderer>()->updateMesh();

	if (!keepOld)
	{
		for (size_t i = 1; i < objectTree->selectedItems.size(); i++)
			delete objectTree->selectedItems[i];
	}
	objectTree->selectedItems.erase(objectTree->selectedItems.begin() + 1, objectTree->selectedItems.end());
	objectTree->update();
	cacheSelection = true;
}



void TienEdit::sortScene()
{
	tien.scene.fortree([](vrlib::tien::Node* node)
	{
		node->sortChildren();
	});

	std::set<std::string> ids;
	tien.scene.fortree([&ids](vrlib::tien::Node* node)
	{
		if (ids.find(node->guid) != ids.end())
			node->guid = vrlib::util::getGuid();
		ids.insert(node->guid);
	});





	objectTree->update();
}