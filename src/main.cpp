#include "Game.h"

int main(int argc, char* args[])
{
    srand(std::time(nullptr));

    Game game;
    game.Run();

    return 0;
}