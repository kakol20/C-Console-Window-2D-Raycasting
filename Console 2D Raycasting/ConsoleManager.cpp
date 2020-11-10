#include "ConsoleManager.h"

void ConsoleManager::Initialise(int screenWidth, int screenHeight)
{
	// Create screen buffer
	screen = new wchar_t[screenWidth * screenHeight];
	hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
	SetConsoleActiveScreenBuffer(hConsole);

	this->screenHeight = screenHeight;
	this->screenWidth = screenWidth;

	for (int x = 0; x < this->screenWidth; x++)
	{
		for (int y = 0; y < this->screenHeight; y++)
		{
			int index = y * this->screenWidth + x;
			screen[index] = ' ';
		}
	}
}

void ConsoleManager::Draw()
{
	screen[screenWidth * screenHeight - 1] = '\0';
	WriteConsoleOutputCharacter(hConsole, screen, screenWidth * screenHeight, { 0,0 }, &dwBytesWritten);
}
