#include "GameManager.h"
#include "ConsoleManager.h"
int main()
{
	TheConsole::Instance()->Initialise(200, 60);
	TheGame::Instance()->Initialise();

	while (TheGame::Instance()->MainLoop())
	{
		TheConsole::Instance()->Draw();
	}

	return 0;
}