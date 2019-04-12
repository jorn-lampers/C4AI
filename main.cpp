#include "C4Bot.h"
#include "C4Abstract.h"

int main()
{
    C4Bot bot;
    bot.run();

    return 0;
}

void test()
{
    Player X = Player::X;
    Player O = Player::O;
    Player N = Player::None;

    std::array<std::array<Player, 7>, 6> test;
    //         0  1  2  3  4  5  6
    test[0] = {N, N, N, N, N, N, N};
    test[1] = {N, N, N, N, N, N, N};
    test[2] = {N, N, N, X, X, X, N};
    test[3] = {N, N, N, X, X, X, N};
    test[4] = {N, N, N, X, X, X, N};
    test[5] = {N, N, N, N, N, N, N};

    auto traps = C4Abstract::LocateTraps(test);

}
