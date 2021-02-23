#include <thread>
#include <iostream>
#include <conio.h>
#include <time.h>
#include <Windows.h>
#include <cmath>

#define MAP_WIDTH 49
#define MAP_HEIGHT 25
#define MAX_ROCKS 20
#define SHOOTER_Y 2
#define ROCK_MOVE_TIME 1500
#define FRAMES_PER_SECOND 40

enum shooter_movement
{
    STOP = 0,
    LEFT,
    RIGHT
} cur_direction;

enum map_objects
{
    EMPTY = 0,
    ROCK,
    BULLET,
    DEBRIS,
    SHOOTER_HEAD,
    SHOOTER_BODY
} map_matrix[MAP_HEIGHT][MAP_WIDTH];

bool is_game_exit;
bool is_game_over;
bool is_game_paused;

COORD shooter;
COORD game_bar;
COORD rocks[MAX_ROCKS];
COORD bullets[MAX_ROCKS];
COORD debris[MAX_ROCKS];

int num_bullets = 5;
int score;

// Utility Functions

void overwriteMapArea(HANDLE h_console)
{
    COORD map_start;
    map_start.X = 1;
    map_start.Y = 1;
    SetConsoleCursorPosition(h_console, map_start);
}

void cls(HANDLE h_console)
{
    // CONSOLE_SCREEN_BUFFER_INFO csbi;
    COORD console_top_left;
    console_top_left.X = 0;
    console_top_left.Y = 0;
    SetConsoleCursorPosition(h_console, console_top_left);
    // GetConsoleScreenBufferInfo(h_console, &csbi);

    //This loop limit obtained by trial and error
    //TODO: Find proper number
    for (int i = 0; i < MAP_WIDTH * 2 * (MAP_HEIGHT + 2); i++)
    {
        std::cout << ' ';
    }
    // std::cout << csbi.dwSize.X << "   " << csbi.dwSize.Y;
    SetConsoleCursorPosition(h_console, console_top_left);
}

void ShowConsoleCursor(bool show_flag, HANDLE h_console)
{
    CONSOLE_CURSOR_INFO cursor_info;

    GetConsoleCursorInfo(h_console, &cursor_info);
    cursor_info.bVisible = show_flag; // set the cursor visibility
    SetConsoleCursorInfo(h_console, &cursor_info);
}

// Game Motion, Object Management and Physics

void rockMotion()
{
    while (!is_game_over)
    {
        Sleep(ROCK_MOVE_TIME);
        for (int i = 0; i < MAX_ROCKS; i++)
        {
            if (rocks[i].X != -1)
            {
                if (map_matrix[rocks[i].Y + 1][rocks[i].X] == BULLET)
                {
                    score += 10;
                    if (num_bullets < 19)
                    {
                        num_bullets += 2;
                    }
                    map_matrix[rocks[i].Y][rocks[i].X] = EMPTY;
                    map_matrix[rocks[i].Y + 1][rocks[i].X] = DEBRIS;
                    for (int j = 0; j < MAX_ROCKS; j++)
                    {
                        if (debris[j].X == -1)
                        {
                            debris[j].X = rocks[i].X;
                            debris[j].Y = rocks[i].Y + 1;
                        }
                    }
                    for (int j = 0; j < MAX_ROCKS; j++)
                    {
                        if (rocks[i].Y + 1 == bullets[j].Y && rocks[i].X == bullets[j].X)
                        {
                            bullets[j].X = -1;
                            break;
                        }
                    }
                    rocks[i].X = -1;
                }
                else if (map_matrix[rocks[i].Y + 1][rocks[i].X] == SHOOTER_HEAD)
                {
                    map_matrix[rocks[i].Y][rocks[i].X] = EMPTY;
                    rocks[i].X = -1;
                    score += 50;
                    if (num_bullets < 16)
                    {
                        num_bullets += 5;
                    }
                    else
                    {
                        num_bullets = 20;
                    }
                }
                else
                {
                    if (rocks[i].Y + 1 == MAP_HEIGHT)
                    {
                        is_game_over = true;
                    }
                    else
                    {
                        map_matrix[rocks[i].Y + 1][rocks[i].X] = ROCK;
                    }
                    map_matrix[rocks[i].Y][rocks[i].X] = EMPTY;
                    rocks[i].Y++;
                }
            }
        }
    }
}

void movementAndCollisionDetection()
{
    switch (cur_direction)
    {
    case STOP:
        break;

    case LEFT:
        map_matrix[shooter.Y][shooter.X] = EMPTY;
        map_matrix[shooter.Y + 1][shooter.X] = EMPTY;
        if (shooter.X > 0)
        {
            shooter.X--;
        }
        else
        {
            shooter.X = MAP_WIDTH - 1;
        }
        if (map_matrix[shooter.Y][shooter.X] == ROCK || map_matrix[shooter.Y + 1][shooter.X] == ROCK)
        {
            score += 50;
            if (num_bullets < 16)
            {
                num_bullets += 5;
            }
            else
            {
                num_bullets = 20;
            }

            for (int i = 0; i < MAX_ROCKS; i++)
            {
                if (rocks[i].X == shooter.X && (rocks[i].Y == shooter.Y || rocks[i].Y == shooter.Y + 1))
                {
                    rocks[i].X = -1;
                    break;
                }
            }
        }
        map_matrix[shooter.Y][shooter.X] = SHOOTER_HEAD;
        map_matrix[shooter.Y + 1][shooter.X] = SHOOTER_BODY;
        break;

    case RIGHT:
        map_matrix[shooter.Y][shooter.X] = EMPTY;
        map_matrix[shooter.Y + 1][shooter.X] = EMPTY;
        if (shooter.X < MAP_WIDTH)
        {
            shooter.X++;
        }
        else
        {
            shooter.X = 0;
        }
        if (map_matrix[shooter.Y][shooter.X] == ROCK || map_matrix[shooter.Y + 1][shooter.X] == ROCK)
        {
            score += 50;
            if (num_bullets < 16)
            {
                num_bullets += 5;
            }
            else
            {
                num_bullets = 20;
            }
            for (int i = 0; i < MAX_ROCKS; i++)
            {
                if (rocks[i].X == shooter.X && (rocks[i].Y == shooter.Y || rocks[i].Y == shooter.Y + 1))
                {
                    rocks[i].X = -1;
                    break;
                }
            }
        }
        map_matrix[shooter.Y][shooter.X] = SHOOTER_HEAD;
        map_matrix[shooter.Y + 1][shooter.X] = SHOOTER_BODY;
        break;

    default:
        break;
    }

    for (int i = 0; i < MAX_ROCKS; i++)
    {
        if (bullets[i].X != -1)
        {
            if (map_matrix[bullets[i].Y - 1][bullets[i].X] == ROCK)
            {
                score += 10;
                if (num_bullets < 19)
                {
                    num_bullets += 2;
                }
                map_matrix[bullets[i].Y][bullets[i].X] = EMPTY;
                map_matrix[bullets[i].Y - 1][bullets[i].X] = DEBRIS;
                for (int j = 0; j < MAX_ROCKS; j++)
                {
                    if (debris[j].X == -1)
                    {
                        debris[j].X = bullets[i].X;
                        debris[j].Y = bullets[i].Y - 1;
                    }
                }
                for (int j = 0; j < MAX_ROCKS; j++)
                {
                    if (rocks[j].Y == bullets[i].Y - 1 && rocks[j].X == bullets[i].X)
                    {
                        rocks[j].X = -1;
                        break;
                    }
                }
                bullets[i].X = -1;
            }
            else
            {
                bullets[i].Y--;
                if (bullets[i].Y > -1)
                {
                    map_matrix[bullets[i].Y][bullets[i].X] = BULLET;
                    map_matrix[bullets[i].Y + 1][bullets[i].X] = EMPTY;
                }
                else
                {
                    map_matrix[bullets[i].Y + 1][bullets[i].X] = EMPTY;
                    bullets[i].X = -1;
                }
            }
        }
    }
}

void createBullet()
{
    if (num_bullets > 0)
    {
        num_bullets--;
        for (int i = 0; i < MAX_ROCKS; i++)
        {
            if (bullets[i].X == -1)
            {
                bullets[i].X = shooter.X;
                bullets[i].Y = shooter.Y - 1;
                if (map_matrix[bullets[i].Y][bullets[i].X] == ROCK)
                {
                    for (int i = 0; i < MAX_ROCKS; i++)
                    {
                        if (debris[i].X == -1)
                        {
                            debris[i].X = bullets[i].X;
                            debris[i].Y = bullets[i].Y;
                        }
                    }
                    map_matrix[bullets[i].Y][bullets[i].X] = DEBRIS;
                    bullets[i].X = -1;

                    if (num_bullets < 19)
                    {
                        num_bullets += 2;
                    }
                }
                else
                {
                    map_matrix[bullets[i].Y][bullets[i].X] = BULLET;
                }
                return;
            }
        }
    }
}

void createRock()
{
    int rock_gen_speed = 50;
    while (!is_game_over)
    {
        if (!is_game_paused)
        {
            if (rock_gen_speed < 200)
            {
                rock_gen_speed++;
            }

            for (int i = 0; i < MAX_ROCKS; i++)
            {
                if (rocks[i].X == -1)
                {
                    rocks[i].X = rand() % MAP_WIDTH;
                    rocks[i].Y = 0;
                    if (map_matrix[0][rocks[i].X] == EMPTY)
                    {
                        map_matrix[0][rocks[i].X] = ROCK;
                        break;
                    }
                    else
                    {
                        rocks[i].X = -1;
                    }
                }
            }

            for (int i = 0; i < MAX_ROCKS; i++)
            {
                if (debris[i].X != -1)
                {
                    map_matrix[debris[i].Y][debris[i].X] = EMPTY;
                    debris[i].X = -1;
                }
            }
            Sleep((1000 / rock_gen_speed) * 100);
        }
    }
}

// Render Functions

void setup()
{
    game_bar.X = 0;
    game_bar.Y = MAP_HEIGHT + 2;
    score = 0;
    for (int i = 0; i < MAX_ROCKS; i++)
    {
        rocks[i].X = -1;
        bullets[i].X = -1;
    }
    for (int i = 0; i < MAP_WIDTH; i++)
    {
        for (int j = 0; j < MAP_HEIGHT; j++)
        {
            map_matrix[j][i] = EMPTY;
        }
    }
    shooter.Y = MAP_HEIGHT - SHOOTER_Y;
    shooter.X = rand() % (MAP_WIDTH - 1) + 1;
    map_matrix[shooter.Y][shooter.X] = SHOOTER_HEAD;
    map_matrix[shooter.Y + 1][shooter.X] = SHOOTER_BODY;

    for (int i = 0; i < MAP_WIDTH / 2 + 2; i++)
    {
        std::cout << "# ";
    }
    std::cout << std::endl;

    for (int j = 0; j < MAP_HEIGHT; j++)
    {
        for (int i = 0; i < MAP_WIDTH + 2; i++)
        {
            if (i == 0 || i == MAP_WIDTH + 1)
            {
                std::cout << '#';
            }
            else if (i == shooter.X && j == shooter.Y)
            {
                std::cout << '^';
            }
            else if (i == shooter.X && j == shooter.Y + 1)
            {
                std::cout << 'O';
            }
            else
            {
                std::cout << ' ';
            }
        }
        std::cout << std::endl;
    }

    for (int i = 0; i < MAP_WIDTH / 2 + 2; i++)
    {
        std::cout << "# ";
    }
    std::cout << std::endl;
    std::cout << "Number of Bullets: " << num_bullets << "  "
              << "Score: " << score;
}

void draw()
{
    // static char rock_list[5] = {'U', 'Y', 'D', '0', 'W'};
    for (int j = 0; j < MAP_HEIGHT; j++)
    {
        for (int i = 0; i < MAP_WIDTH; i++)
        {
            switch (map_matrix[j][i])
            {
            case EMPTY:
                std::cout << ' ';
                break;

            case ROCK:
                std::cout << 'O';
                break;

            case BULLET:
                std::cout << '|';
                break;

            case DEBRIS:
                std::cout << 'X';
                break;

            case SHOOTER_HEAD:
                std::cout << '^';
                break;

            case SHOOTER_BODY:
                std::cout << '0';
                break;

            default:
                break;
            }
        }
        std::cout << std::endl
                  << '#';
    }
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), game_bar);
    std::cout << "Number of Bullets: " << num_bullets << "  "
              << "Score: " << score;
}

int main()
{
    HANDLE h_console = GetStdHandle(STD_OUTPUT_HANDLE);
    is_game_exit = false;
    std::cout << "(:  Press any key to begin! (Other than X) :)" << std::endl;
    std::cout << "---------------Press X to exit---------------";
    char ch = _getch();
    if (ch == 'x' || ch == 'X')
    {
        is_game_exit = true;
    }

    // Randomness by time of start
    srand(time(0));

    while (!is_game_exit)
    {
        ShowConsoleCursor(false, h_console);
        // Clear console and set random initial values
        cls(h_console);
        // Seed for random number generator with randomness by time value transformation function
        setup();
        is_game_over = false;

        // Calling this thread to call a function which will keep generating rocks
        std::thread rock_creation(createRock);
        // detach() thread to run independently in bg
        rock_creation.detach();
        // Use this to pause main thread while this thread finishes execution
        // rock_creation.join();
        std::thread rock_movement(rockMotion);
        rock_movement.detach();

        while (!is_game_over)
        {
            if (_kbhit())
            {
                char ch = _getch();
                switch (ch)
                {
                case 'x':
                case 'X':
                    cur_direction = STOP;
                    cls(h_console);
                    is_game_over = true;
                    break;

                case 'p':
                case 'P':
                    is_game_paused = true;
                    cur_direction = STOP;
                    break;

                case 's':
                case 'S':
                    is_game_paused = false;
                    cur_direction = STOP;
                    break;

                case 'a':
                case 'A':
                    is_game_paused = false;
                    cur_direction = LEFT;
                    break;

                case 'd':
                case 'D':
                    is_game_paused = false;
                    cur_direction = RIGHT;
                    break;

                case ' ':
                    is_game_paused = false;
                    if (num_bullets > 0)
                    {
                        createBullet();
                    }
                    break;

                default:
                    break;
                }
            }
            if (is_game_over)
            {
                break;
            }
            if (!is_game_paused)
            {
                overwriteMapArea(h_console);
                movementAndCollisionDetection();
                draw();
            }
            Sleep(1000 / FRAMES_PER_SECOND);
        }

        // Game Over Screen
        cls(h_console);
        std::cout << "Again?";
        char ch = _getch();
        if (ch == 'x' || ch == 'X')
        {
            is_game_exit = true;
        }
    }
    return 0;
}