
#include "Engone/Engone.h"

#include "UniSync/SyncApp.h"

#include "Engone/Utilities/Utilities.h"

#include "Engone/Utilities/Tracker.h"

#include <shellapi.h>

void runApp(int argc, char** argv) {
	using namespace engone;

	int flag = 0;
	std::string cachePath = "cache.dat";
	for (int i = 0; i < argc;i++) {
		if (flag == 1) {
			cachePath = argv[i];
		}
		if (strcmp(argv[i], "--console") == 0|| strcmp(argv[i], "-c")==0) {
			//flag = 1;
			CreateConsole();
		} else if (strcmp(argv[i], "--file") == 0 || strcmp(argv[i], "-f") == 0) {
			flag = 1;
		}
	}
	{
		engone::Engone engone;
		//unisync::SyncApp* app = engone.createApplication<unisync::SyncApp>(cachePath);
		
		unisync::SyncApp* app = engone.createApplication<unisync::SyncApp>("cache1.dat");
		unisync::SyncApp* app2 = engone.createApplication<unisync::SyncApp>("cache2.dat");

		engone.getStats().setFPSLimit(60);
		engone.getStats().setUPSLimit(40);
		engone.start();
	}
	engone::Sleep(1);
	//std::cin.get();
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow) {
	using namespace engone;

	int argc;
	char** argv;
	ConvertArguments(argc,argv);
	runApp(argc,argv);
	FreeArguments(argc, argv);
	return 0;
}
int main(int argc, char** argv) {
	runApp(argc,argv);
	return 0;
}