#include "Game.hpp"

int main()
{
    Game game;
    if (!game.initialise())
        return -1;
    
    game.run();

    return 1;
}
