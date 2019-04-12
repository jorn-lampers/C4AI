#ifndef C4ABSTRACT_H
#define C4ABSTRACT_H

#include "C4Game.h"

struct Position
{
    int row;
    int column;
    Position(int r, int c) {
        row = r;
        column = c;
    }
};

struct TrappedSlot
{
    Player player = Player::None;           // Player that might profit from the trap
    Position position = Position(-1, -1);   // Position of slot in 4
};

std::ostream &operator<<(std::ostream& os, const Position &pos);

class C4Abstract {

public:
    /// Locates all traps in a game-state.
    /// Arguments:
    /// - state: the state to search
    static std::vector<TrappedSlot> LocateTraps(const State &state);

    /// Examines if slot is trapped by either Player, Both, or None
    static Player GetSlotTrappedByPlayer(const State &state, const Position &slot);

    /// Returns the amount of coins that have been dropped in each column of a game-state
    static std::array<int, 7> GetColumnProgressions(const State &state);
};

#endif
