#include "Game.hpp"

int main(int argv, char** args)
{
    Game game;
    if (!game.initialise())
        return -1;
    
    game.run();

    return 1;
}
