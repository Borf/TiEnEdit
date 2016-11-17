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


int main(int argc, char** argv) {	
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
