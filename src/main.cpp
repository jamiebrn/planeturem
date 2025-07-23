#include "Game.hpp"

int main(int argc, char* argv[])
{
    Game game;
    if (!game.initialise())
        return -1;
    
    game.run();
    game.deinit();

    return 0;
}