#include "App/GameApplication.h"

int WINAPI WinMain(
	HINSTANCE Instance,
	HINSTANCE,
	LPSTR,
	int ShowCommand)
{
	GameApplication Application;
	return Application.Run(Instance, ShowCommand);
}
