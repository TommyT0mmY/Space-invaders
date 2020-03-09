#include <iostream>
#include <Windows.h>
#include <string>
#include <chrono>
#include <vector>

#define HORIZONTAL_SPEED 70
#define VERTICAL_SPEED 50
#define PROJECTILES_SPEED 75
#define ENEMY_PROJECTILES_SPEED 60
#define PARTICLES_SPEED 300
#define DAMAGE_PER_HIT 10
#define ENEMY_SPEED 40

int const ConsoleHeight = 40, ConsoleWidth = 120;

float PlayerX = ConsoleWidth / 2 - 4, PlayerY = ConsoleHeight / 2 - 2;
int health = 100;
int kills = 0;
bool GAME_OVER = false;

std::vector<std::pair<float, float>> projectiles;
std::vector<std::pair<float, float>> enemy_projectiles;
std::vector<std::pair<float, float>> particles;
std::vector<std::pair<float, float>> enemies;

int main()
{
    //Screen Buffer
    wchar_t* screen = new wchar_t[ConsoleWidth * ConsoleHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false; // set the cursor visibility
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    //Timers
    auto tp1 = std::chrono::system_clock::now();
    auto tp2 = std::chrono::system_clock::now();
    unsigned long shootingCooldown = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);

    //Spaceship drawing (do not modify if you don't know what you're doing)
    std::wstring spaceship;
    spaceship += L"<====>  ";
    spaceship += L"  \\ \\   ";
    spaceship += L"([][][])";
    spaceship += L"  / /   ";
    spaceship += L"<====>  ";

    //Enemy drawing (do not modify if you don't know what you're doing)
    std::wstring enemy;
    enemy += L"  /==\\";
    enemy += L"(=|##|";
    enemy += L"  \\==/";

    while (1) 
    {
        //TIMING

        tp2 = std::chrono::system_clock::now();
        std::chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();

        if (!GAME_OVER) {

            //Game Over Check
            if (health < 0) {
                GAME_OVER = true;
            }

            //INPUT

            if (GetAsyncKeyState(VK_UP) || GetAsyncKeyState('W'))
                if (PlayerY > 2)
                    PlayerY -= (VERTICAL_SPEED)*fElapsedTime;

            if (GetAsyncKeyState(VK_DOWN) || GetAsyncKeyState('S'))
                if (PlayerY < 35)
                    PlayerY += (VERTICAL_SPEED)*fElapsedTime;

            if (GetAsyncKeyState(VK_LEFT) || GetAsyncKeyState('A'))
                if (PlayerX > 0)
                    PlayerX -= (HORIZONTAL_SPEED)*fElapsedTime;

            if (GetAsyncKeyState(VK_RIGHT) || GetAsyncKeyState('D'))
                if (PlayerX < 112)
                    PlayerX += (HORIZONTAL_SPEED)*fElapsedTime;

            if (GetAsyncKeyState(VK_SPACE))
            {
                unsigned long currentTimeMillis = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
                if (currentTimeMillis > shootingCooldown)
                {
                    shootingCooldown = currentTimeMillis + 200;
                    projectiles.push_back({ PlayerX + 5, PlayerY + 2 });
                }
            }

            //DRAWING

            //Clearing Buffer
            for (int y = 0; y < ConsoleHeight; y++)
                for (int x = 0; x < ConsoleWidth; x++)
                    screen[y * ConsoleWidth + x] = ' ';

            //Drawing particles
            if (rand() % 300 == 1)
            {
                int number = rand() % 3;
                for (int i = 0; i < number; ++i)
                    particles.push_back({ 119, (rand() % 39) + 1 });
            }
            for (int i = 0; i < particles.size(); ++i)
            {
                if (particles[i].first > 0) {
                    particles[i].first -= PARTICLES_SPEED * fElapsedTime;
                    screen[(int)particles[i].second * ConsoleWidth + (int)particles[i].first] = '.';
                }
                else
                    particles.erase(particles.begin() + i);
            }

            //Drawing projectiles
            for (int i = 0; i < projectiles.size(); ++i)
            {
                if (projectiles[i].first < 118)
                {
                    projectiles[i].first += PROJECTILES_SPEED * fElapsedTime;
                    screen[(int)projectiles[i].second * ConsoleWidth + (int)projectiles[i].first] = '#';
                    screen[((int)projectiles[i].second * ConsoleWidth + (int)projectiles[i].first) + 1] = '>';
                }
                else
                    projectiles.erase(projectiles.begin() + i);
            }

            //Collision detection
            std::vector<int> enemies_to_erase, projectiles_to_erase;
            
            for (int i = 0; i < projectiles.size(); ++i)
                for (int j = 0; j < enemies.size(); ++j) {
                    if (abs(enemies[j].first + 3 - projectiles[i].first) < 3 && abs(enemies[j].second + 1 - projectiles[i].second) < 2) {
                        kills++;
                        projectiles_to_erase.push_back(i);
                        enemies_to_erase.push_back(j);
                    }
                }
            for (int i = 0; i < enemies_to_erase.size(); ++i)
                enemies.erase(enemies.begin() + i);
            for (int i = 0; i < projectiles_to_erase.size(); ++i)
                projectiles.erase(projectiles.begin() + i);

            enemies_to_erase.clear();
            projectiles_to_erase.clear();

            //Drawing enemy projectiles
            for (int i = 0; i < enemy_projectiles.size(); ++i)
            {
                if (enemy_projectiles[i].first > 0)
                {
                    enemy_projectiles[i].first -= ENEMY_PROJECTILES_SPEED * fElapsedTime;
                    screen[((int)enemy_projectiles[i].second * ConsoleWidth + (int)enemy_projectiles[i].first) + 1] = '#';
                    screen[(int)enemy_projectiles[i].second * ConsoleWidth + (int)enemy_projectiles[i].first] = '<';
                    if (abs(PlayerX + 4 - enemy_projectiles[i].first) < 2 && abs(PlayerY + 2 - enemy_projectiles[i].second) < 2) //collision check
                    {
                        health -= DAMAGE_PER_HIT;
                        enemy_projectiles.erase(enemy_projectiles.begin() + i);
                    }
                }
                else
                    enemy_projectiles.erase(enemy_projectiles.begin() + i);
            }

            //Drawing enemies
            if (rand() % 1500 == 1) //spawning
            {
                int y = (rand() % 35) + 2, x = 114;
                bool spawn_flag = true;
                //Checking possible collisions
                for (int i = 0; i < enemies.size(); ++i) {
                    if (abs(enemies[i].first + 3 - x) < 6 && abs(enemies[i].second + 2 - y) < 6)
                        spawn_flag = false;
                }
                if (spawn_flag)
                    enemies.push_back({ x, y });
            }

            for (int i = 0; i < enemies.size(); ++i) {
                if (abs(PlayerX + 4 - enemies[i].first) < 2 && abs(PlayerY + 2 - enemies[i].second) < 3) {
                    health -= DAMAGE_PER_HIT;
                    enemies_to_erase.push_back(i);
                }
                if (enemies[i].first > 0) {
                    if (rand() % 3000 == 1) { //shooting projectiles
                        enemy_projectiles.push_back({ enemies[i].first, enemies[i].second + 1 });
                    }
                    enemies[i].first -= ENEMY_SPEED * fElapsedTime;
                }
                else
                    enemies_to_erase.push_back(i);

                for (int y = 0; y < 3; y++)
                    for (int x = 0; x < 6; x++)
                    {
                        if (enemy[y * 6 + x] != ' ') {
                            screen[(int)(y + enemies[i].second) * ConsoleWidth + (int)(x + enemies[i].first)] = enemy[y * 6 + x];
                        }
                    }
            }
            for (int i = 0; i < enemies_to_erase.size(); ++i) {
                enemies.erase(enemies.begin() + i);
            }

            enemies_to_erase.clear();

            //Drawing Spaceship
            for (int y = 0; y < 5; y++)
                for (int x = 0; x < 8; x++)
                {
                    if (spaceship[y * 8 + x] != ' ') {
                        screen[(y + (int)PlayerY) * ConsoleWidth + (x + (int)PlayerX)] = spaceship[y * 8 + x];
                    }
                }

            //Drawing Score
            for (int x = 0; x < ConsoleWidth; ++x)
                screen[x] = '#';
            std::string score_string = " Health: " + std::to_string(health) + " Kills: " + std::to_string(kills) + " ";
            int i = 0;
            for (int x = ConsoleWidth / 2 - score_string.size() / 2; x < ConsoleWidth / 2 + score_string.size() / 2; ++x) {
                screen[x] = score_string[i];
                ++i;
            }
        }
        else //GAME OVER
        {
        if (GetAsyncKeyState(VK_ESCAPE)) {
            PlayerX = ConsoleWidth / 2 - 4, PlayerY = ConsoleHeight / 2 - 2;
            health = 100;
            kills = 0;
            GAME_OVER = false;
        }

            //Clearing Buffer
            for (int y = 0; y < ConsoleHeight; y++)
                for (int x = 0; x < ConsoleWidth; x++)
                    screen[y * ConsoleWidth + x] = ' ';
            
            //Game over text
            std::string game_over_text = "Game Over, Press esc to retry";
            int i = -2;
            for (int x = ConsoleWidth / 2 - game_over_text.size() / 2 - 2; x < ConsoleWidth / 2 + game_over_text.size() / 2 + 3; ++x) {
                screen[((ConsoleHeight / 2) - 2) * ConsoleWidth + x] = '#';
                screen[((ConsoleHeight / 2) + 2) * ConsoleWidth + x] = '#';
                if (game_over_text.size() > i && i >= 0) {
                    screen[(ConsoleHeight / 2) * ConsoleWidth + x] = game_over_text[i];
                }
                ++i;
            }
            for (int delta_y = -1; delta_y < 2; ++delta_y) {
                screen[((ConsoleHeight / 2) + delta_y) * ConsoleWidth + ConsoleWidth / 2 - game_over_text.size() / 2 - 2] = '#';  
                screen[((ConsoleHeight / 2) + delta_y) * ConsoleWidth + ConsoleWidth / 2 + game_over_text.size() / 2 + 2] = '#';
            }
        }

        screen[ConsoleWidth * ConsoleHeight - 1] = '\0';
        WriteConsoleOutputCharacter(hConsole, screen, ConsoleWidth * ConsoleHeight, { 0,0 }, &dwBytesWritten);
    }
}
