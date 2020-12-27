#ifndef CONSOLEMANAGER_H
#define CONSOLEMANAGER_H

#include <Windows.h>

#include "Singleton.h"

class ConsoleManager
{
	friend class Singleton<ConsoleManager>;

public:
	void Initialise(int screenWidth, int screenHeight);

	void Clear();

	void Draw();

	wchar_t* screen = new wchar_t[1];
	HANDLE hConsole = nullptr;
	DWORD dwBytesWritten = 0;

	int screenWidth = 0, screenHeight = 0;

private:
	ConsoleManager() {}
	ConsoleManager(const ConsoleManager&);
	ConsoleManager& operator=(const ConsoleManager&);
};

typedef Singleton<ConsoleManager> TheConsole;
#endif // !CONSOLEMANAGER_H
