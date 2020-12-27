#include "GameManager.h"

void GameManager::Initialise()
{
	m_map += L"################";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"#..............#";
	m_map += L"################";

	m_player.FOV = DegreesToRadians(35.0f);

	m_player.x = 1.0f;
	m_player.y = 1.0f;

	m_player.angle = DegreesToRadians(45.0f);
	m_player.deltaX = (float)sinf(m_player.angle);
	m_player.deltaY = (float)cosf(m_player.angle);

	GenerateShadingChars();
}

bool GameManager::MainLoop()
{

	// clear screen
	TheConsole::Instance()->Clear();


	m_tp2 = std::chrono::system_clock::now();
	std::chrono::duration<float> deltaTimeCast = m_tp2 - m_tp1;
	m_tp1 = m_tp2;

	m_deltaTime = deltaTimeCast.count();

	ControlPlayer();
	CalculateRays(0.01f);
	DrawMap();

	// show stats
	swprintf_s(TheConsole::Instance()->screen, TheConsole::Instance()->screenWidth, L"FPS:%3.2f \0", 1.0f / m_deltaTime);

	return true;
}

void GameManager::GenerateShadingChars()
{
	// 10 levels
	//m_shades.push_back(' ');
	/*m_shades.push_back('.');
	m_shades.push_back(':');
	m_shades.push_back('-');
	m_shades.push_back('=');
	m_shades.push_back('+');
	m_shades.push_back('*');
	m_shades.push_back('#');
	m_shades.push_back('%');
	m_shades.push_back('@');*/

	// 66 levels
	std::string levels = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/|()1{}[]?-_+~<>i!lI;:,`'.";

	for (int i = 0; i < levels.size(); i++)
	{
		m_shades.push_back(levels[i]);
	}
}

void GameManager::ControlPlayer()
{
	// Rotation
	float rotationSpeed = DegreesToRadians(45.0f) * m_deltaTime;

	if (GetAsyncKeyState((unsigned short)'Q') * 0x8000)
	{
		m_player.angle -= rotationSpeed;
	}

	if (GetAsyncKeyState((unsigned short)'E') * 0x8000)
	{
		m_player.angle += rotationSpeed;
	}

	if (m_player.angle <= DegreesToRadians(0.0f)) m_player.angle = DegreesToRadians(359.0f);
	if (m_player.angle >= DegreesToRadians(360)) m_player.angle = 0.0f;

	// Movement
	m_player.deltaX = (float)sinf(m_player.angle);
	m_player.deltaY = (float)cosf(m_player.angle);

	float movementSpeed = 1 * m_deltaTime;

	float newX = m_player.x, newY = m_player.y;

	if (GetAsyncKeyState((unsigned short)'W') * 0x8000)
	{
		newX += m_player.deltaX * movementSpeed;
		newY += m_player.deltaY * movementSpeed;
	}

	if (GetAsyncKeyState((unsigned short)'S') * 0x8000)
	{
		newX -= m_player.deltaX * movementSpeed;
		newY -= m_player.deltaY * movementSpeed;
	}

	if (m_map.c_str()[PositionToIndex((int)newX, (int)newY, m_mapWidth)] != '#')
	{
		m_player.x = newX;
		m_player.y = newY;
	}
}

void GameManager::CalculateRays(float step)
{
	//float maxDistance = sqrtf((float)(m_mapHeight * m_mapHeight) + (float)(m_mapWidth * m_mapWidth));
	float maxDistance = Max((float)m_mapHeight, (float)m_mapWidth);

	for (int x = 0; x < TheConsole::Instance()->screenWidth; x++)
	{
		bool hitWall = false;

		// calculate rays
		float rayAngle = (m_player.angle - m_player.FOV / 2.0f) + ((float)x / (float)TheConsole::Instance()->screenWidth) * m_player.FOV;

		float rayDeltaX = sinf(rayAngle);
		float rayDeltaY = cosf(rayAngle);
		float distanceToHit = 0;

		int wallX = 0;
		int wallY = 0;

		bool boundary = false;

		while (!hitWall && distanceToHit < maxDistance)
		{
			distanceToHit += step;

			wallX = (int)roundf(m_player.x + rayDeltaX * distanceToHit);
			wallY = (int)roundf(m_player.y + rayDeltaY * distanceToHit);

			// test if ray is out of bounds
			if (wallX < 0 || wallY < 0 || wallX >= m_mapWidth || wallY >= m_mapHeight)
			{
				hitWall = true;
			}
			else
			{
				if (m_map.c_str()[PositionToIndex(wallX, wallY, m_mapWidth)] == '#')
				{
					hitWall = true;
				}
			}
		}

		// Fix fisheye
		distanceToHit *= cosf(rayAngle - m_player.angle);

		// Calculate length of wall
		float minHeight = (float)TheConsole::Instance()->screenHeight / 4.0f;

		int height = (int)MapRange(distanceToHit, 0.0f, maxDistance, (float)TheConsole::Instance()->screenHeight, 0.0f);
		int startY = (TheConsole::Instance()->screenHeight - height) / 2;
		int endY = startY + height;

		for (int y = 0; y < TheConsole::Instance()->screenHeight; y++)
		{
			int index = PositionToIndex(x, y, TheConsole::Instance()->screenWidth);

			if (y >= startY && y <= endY)
			{
				// shade wall
				float distanceToWallX = abs(m_player.x - (float)wallX);
				float distanceToWallY = abs(m_player.y - (float)wallY);
				float distanceToWall = (float)sqrt(((double)distanceToWallX * distanceToWallX) + ((double)distanceToWallY * distanceToWallY));

				short shade = ' ';

				// old style
				/*float factor = MapRange(distanceToWall, 0.0f, maxDistance, 3.0f, 0.0f);
				if (factor == 0.0f)
				{
					shade = ' ';
				}
				else if (factor < 1.0f)
				{
					shade = 0x2592;
				}
				else if (factor < 2.0f)
				{
					shade = 0x2593;
				}
				else
				{
					shade = 0x2588;
				}*/

				// new style
				int factor = (int)MapRange(distanceToWall, 0.0f, maxDistance, 0.0f, (float)m_shades.size() - 1.0f);
				factor = factor < 0 ? 0 : factor;
				factor = factor >= (int)m_shades.size() ? (int)m_shades.size() - 1 : factor;
				shade = m_shades[factor];

				// test for edge of blocks
				std::vector<std::pair<float, float>> p; // distance and dot

				for (int tx = 0; tx < 2; tx++)
				{
					for (int ty = 0; ty < 2; ty++)
					{
						// angle of corner to camera
						float vy = (float)wallY + ty - m_player.y;
						float vx = (float)wallX + tx - m_player.x;

						float d = (float)sqrt(((double)vx * (double)vx) + ((double)vy * (double)vy));
						float dot = (rayDeltaX * vx / d) + (rayDeltaY * vy / d);

						p.push_back(std::make_pair(d, dot));
					}
				}

				std::sort(p.begin(), p.end(), [](const std::pair<float, float>& left, const std::pair<float, float>& right) { return left.first < right.first; });

				float bound = 0.01f / 7.0f;

				if (acos(p.at(0).second) < bound) boundary = true;
				if (acos(p.at(1).second) < bound) boundary = true;
				if (acos(p.at(2).second) < bound) boundary = true;

				//if (boundary) shade = ' ';

				TheConsole::Instance()->screen[index] = shade;
			}
			else if (y > endY)
			{
				// shade floor

				float factor = MapRange((float)y, (float)TheConsole::Instance()->screenHeight, (float)TheConsole::Instance()->screenHeight / 2.0f, 4.0f, 0.0f);
				short shade = ' ';

				if (factor < 1.0f) // far away
				{
					shade = ' ';
				}
				else if (factor < 2.0f)
				{
					shade = 0x2592;
				}
				else if (factor < 3.0f)
				{
					shade = 0x2593;
				}
				else
				{
					shade = 0x2588; // close
				}
				TheConsole::Instance()->screen[index] = shade;
			}
			else
			{
				// shade ceiling
				TheConsole::Instance()->screen[index] = ' ';
			}
		}
	}
}

void GameManager::DrawMap()
{
	for (int x = 0; x < m_mapWidth; x++)
	{
		for (int y = 0; y < m_mapHeight; y++)
		{
			int mapIndex = PositionToIndex(x, y, m_mapWidth);
			int screenIndex = PositionToIndex(x, y + 1, TheConsole::Instance()->screenWidth);

			TheConsole::Instance()->screen[screenIndex] = m_map[mapIndex];
		}
	}
	// Place Player
	int playerX = (int)m_player.x;
	int playerY = (int)m_player.y;
	int directionX = playerX + (int)(ceilf(m_player.deltaX));
	int directionY = playerY + (int)(ceilf(m_player.deltaY));

	TheConsole::Instance()->screen[PositionToIndex(directionX, directionY + 1, TheConsole::Instance()->screenWidth)] = 'o';
	TheConsole::Instance()->screen[PositionToIndex(playerX, playerY + 1, TheConsole::Instance()->screenWidth)] = 'P';
}

float GameManager::DegreesToRadians(const float deg)
{
	return deg * ((float)PI / 180.0f);
}

int GameManager::PositionToIndex(const int x, const int y, const int arrayWidth)
{
	return y * arrayWidth + x;
}

float GameManager::MapRange(const float val, const float fromMin, const float fromMax, const float toMin, const float toMax)
{
	return (val - fromMin) * (toMax - toMin) / (fromMax - fromMin) + toMin;
}

float GameManager::Max(const float x, const float y)
{
	return x > y ? x : y;
}