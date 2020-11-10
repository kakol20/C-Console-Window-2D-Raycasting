#include "GameManager.h"

void GameManager::Initialise()
{
    m_map += L"################";
    m_map += L"#..............#";
    m_map += L"#.......########";
    m_map += L"#..............#";
    m_map += L"#.......##.....#";
    m_map += L"#.......##.....#";
    m_map += L"#..............#";
    m_map += L"###............#";
    m_map += L"##.............#";
    m_map += L"#........#######";
    m_map += L"#........#.....#";
    m_map += L"#........#.....#";
    m_map += L"#..............#";
    m_map += L"#........#######";
    m_map += L"#..............#";
    m_map += L"################";

    m_player.FOV = DegreesToRadians(35.0f);

    m_player.x = 1.0f;
    m_player.y = 1.0f;

    m_player.angle = DegreesToRadians(45.0f);
    m_player.deltaX = (float)sinf(m_player.angle);
    m_player.deltaY = (float)cosf(m_player.angle);
}

bool GameManager::MainLoop()
{
    m_tp2 = std::chrono::system_clock::now();
    std::chrono::duration<float> deltaTimeCast = m_tp2 - m_tp1;
    m_tp1 = m_tp2;

    m_deltaTime = deltaTimeCast.count();

    ControlPlayer();
    CalculateRays();
    DrawMap();

    return true;
}

void GameManager::ControlPlayer()
{
    // Rotation
    float rotationSpeed = DegreesToRadians(45.0f) * m_deltaTime;

    if (GetAsyncKeyState((unsigned short)'A') * 0x8000)
    {
        m_player.angle -= rotationSpeed;
    }

    if (GetAsyncKeyState((unsigned short)'D') * 0x8000)
    {
        m_player.angle += rotationSpeed;
    }

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

void GameManager::CalculateRays()
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
        float distanceToWall = 0;
        float step = 0.01f;

        int testX = 0;
        int testY = 0;

        while (!hitWall && distanceToWall < maxDistance)
        {
            distanceToWall += step;

            testX = (int)roundf(m_player.x + rayDeltaX * distanceToWall);
            testY = (int)roundf(m_player.y + rayDeltaY * distanceToWall);

            // test if ray is out of bounds
            if (testX < 0 || testY < 0 || testX >= m_mapWidth || testY >= m_mapHeight)
            {
                hitWall = true;
            }
            else
            {
                if (m_map.c_str()[PositionToIndex(testX, testY, m_mapWidth)] == '#')
                {
                    hitWall = true;
                }
            }
        }

        // Fix fisheye
        distanceToWall *= cosf(rayAngle - m_player.angle);

        // Calculate length of wall
        float minHeight = (float)TheConsole::Instance()->screenHeight / 4.0f;

        int height = (int)MapRange(distanceToWall, 0.0f, maxDistance, (float)TheConsole::Instance()->screenHeight, 1.0f);
        int startY = (TheConsole::Instance()->screenHeight - height) / 2;
        int endY = startY + height;

        for (int y = 0; y < TheConsole::Instance()->screenHeight; y++)
        {
            int index = PositionToIndex(x, y, TheConsole::Instance()->screenWidth);

            if (y >= startY && y <= endY)
            {
                // shade wall
                float factor = MapRange(distanceToWall, 0.0f, maxDistance, 3.0f, 0.0f);
                short shade = ' ';

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
                }

                // test for edge of blocks
                std::vector<std::pair<float, float>> p; // distance and dot

                for (int tx = 0; tx < 2; tx++)
                {
                    for (int ty = 0; ty < 2; ty++)
                    {
                        // angle of corner to camera
                        float vy = (float)testY + ty - m_player.y;
                        float vx = (float)testX + tx - m_player.x;

                        float d = (float)sqrt(((double)vx * (double)vx) + ((double)vy * (double)vy));
                        float dot = (rayDeltaX * vx / d) + (rayDeltaY * vy / d);

                        p.push_back(std::make_pair(d, dot));
                    }
                }

                std::sort(p.begin(), p.end(), [](const std::pair<float, float>& left, const std::pair<float, float>& right) { return left.first < right.first; });

                bool boundary = false;
                float bound = 0.01f / 7.0f;

                if (acosf(p.at(0).second) < bound) boundary = true;
                if (acosf(p.at(1).second) < bound) boundary = true;
                if (acosf(p.at(2).second) < bound) boundary = true;

                if (boundary) shade = ' ';

                TheConsole::Instance()->screen[index] = shade;
            }
            else if (y > endY)
            {
                // shade floor

                float factor = MapRange((float)y, (float)TheConsole::Instance()->screenHeight, (float)TheConsole::Instance()->screenHeight / 2.0f, 0.0f, 4.0f);
                short shade = ' ';
                if (factor < 1.0f)
                {
                    shade = '#';
                }
                else if (factor < 2.0f)
                {
                    shade = 'x';
                }
                else if (factor < 3.0f)
                {
                    shade = '.';
                }
                else
                {
                    shade = '-';
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
            int mapIndex = PositionToIndex(x, m_mapHeight - y - 1, m_mapWidth);
            int screenIndex = PositionToIndex(x, m_mapHeight - y - 1, TheConsole::Instance()->screenWidth);

            TheConsole::Instance()->screen[screenIndex] = m_map[mapIndex];
        }
    }
    // Place Player
    int playerX = (int)m_player.x;
    int playerY = (int)m_player.y;
    int directionX = playerX + (int)(ceilf(m_player.deltaX));
    int directionY = playerY + (int)(ceilf(m_player.deltaY));

    playerY = m_mapHeight - playerY - 1;
    directionY = m_mapHeight - directionY - 1;

    TheConsole::Instance()->screen[PositionToIndex(directionX, directionY, TheConsole::Instance()->screenWidth)] = 'o';
    TheConsole::Instance()->screen[PositionToIndex(playerX, playerY, TheConsole::Instance()->screenWidth)] = 'P';
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
