#include <VrLib/Kernel.h>
#include <vrlib/json.h>
#include <vrlib/ServerConnection.h>
#include <VrLib/Log.h>

#include <sys/stat.h>
#include <direct.h>

#include "TienEdit.h"
#include <iostream>
#include <cstdlib>
#include <string>
#include <regex>

extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

HWND GetConsoleHwnd()
{
	HWND hwndFound;
	char TempWindowTitle[1024];
	char WindowTitle[1024];
	GetConsoleTitleA(WindowTitle, 1024);
	sprintf_s(TempWindowTitle, 1024, "%d/%d", GetTickCount(), GetCurrentProcessId());
	SetConsoleTitleA(TempWindowTitle);
	Sleep(10);
	hwndFound = FindWindowA(NULL, TempWindowTitle);
	SetConsoleTitleA(WindowTitle);
	return(hwndFound);
}
RECT screenRect;
static int posX = -1;
static int posY = -1;
BOOL CALLBACK enumProc(HMONITOR monitor, HDC hdc, LPRECT rect, LPARAM param)
{
	MONITORINFO monitorInfo;
	monitorInfo.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &monitorInfo);
	if ((monitorInfo.dwFlags & MONITORINFOF_PRIMARY) == 0)
	{
		posX = rect->left;
		posY = rect->top;
	}
	return TRUE;
}
void fixConsole()
{
#ifdef WIN32
	if (GetSystemMetrics(80) > 1)
	{
		EnumDisplayMonitors(NULL, NULL, enumProc, NULL);
		SetWindowPos(GetConsoleHwnd(), GetConsoleHwnd(), posX, posY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	}
#endif
}

int main(int argc, char** argv) {	
	fixConsole();
	vrlib::Kernel* kernel = vrlib::Kernel::getInstance();

	std::string filename;

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "--config") == 0)
		{
			i++;
			kernel->loadConfig(argv[i]);
		}
		else
		{
			filename = argv[i];
		}

	}
	TienEdit* editor = new TienEdit(filename);

	kernel->setApp(editor);
	kernel->start();
	return 0;
}
