#include "application.h"

int Main()
{
	Application app;
	app.Init(L"Sun from Earth", 1150, 1000);
	app.Run();

	return 0;
}

int WINAPI WinMain(HINSTANCE __in hInstance, HINSTANCE __in_opt hPrevInstance, LPSTR szCmdLine, int nCmdShow)
{
	return Main();
}

int main(int argc, char* argv[])
{
	return Main();
}