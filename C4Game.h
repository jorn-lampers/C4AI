#ifndef C4_H
#define C4_H

#include <random>
#include <array>

enum class Player
{
    None, X, O, Both
};

using Move = int;
using State = std::array<std::array<Player,7>,6>;

std::ostream &operator<<(std::ostream& os, const Player &p);
std::ostream &operator<<(std::ostream& os, const State &s);
Player getCurrentPlayer(const State &state);
State doMove(const State &state, const Move &m);
Player getWinner(const State &state);
std::vector<Move> getMoves(const State &state);

#endif // C4_H

