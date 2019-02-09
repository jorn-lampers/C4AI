#include <iostream>
#include <vector>
#include "C4Game.h"
#include "C4AI.h"

int main()
{
/*
    Player N = Player::None;
    Player X = Player::X;
    Player O = Player::O;

    State s = {{
            {N, N, N, N, N, N, N},
            {N, N, N, N, N, N, N},
            {N, N, N, N, N, N, N},
            {N, N, N, N, O, N, N},
            {N, N, N, O, N, N, N},
            {N, O, O, O, N, O, N}
    }};

    C4AI::RateByPotentialTraps(s, X);

    */

    C4Bot bot;
    bot.run();

    return 0;
}
