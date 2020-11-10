#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <Windows.h>
#include <string>
#include <algorithm>
#include <chrono>
#include <vector>

#include "Singleton.h"
#include "ConsoleManager.h"

#define PI 3.14159265

struct Player
{
	float x;
	float y;
	float angle;
	float FOV;
	float deltaX;
	float deltaY;
};

class GameManager
{
	friend class Singleton<GameManager>;

public:

	void Initialise();

	bool MainLoop();

private:
	void ControlPlayer();
	void CalculateRays();
	void DrawMap();

	float DegreesToRadians(const float deg);
	int PositionToIndex(const int x, const int y, const int arrayWidth);
	float MapRange(const float val, const float fromMin, const float fromMax, const float toMin, const float toMax);
	float Max(const float x, const float y);

private:
	Player m_player = { 0 };

	int m_mapWidth = 16;
	int m_mapHeight = 16;
	std::wstring m_map;

	// Delta Time
	std::chrono::time_point<std::chrono::system_clock> m_tp1 = std::chrono::system_clock::now();
	std::chrono::time_point<std::chrono::system_clock> m_tp2 = std::chrono::system_clock::now();
	float m_deltaTime = 0.0f;

private:
	GameManager() {}
	GameManager(const GameManager&);
	GameManager& operator=(const GameManager&);
};

typedef Singleton<GameManager> TheGame;

#endif // !GAMEMANAGER_H
