//
// Created by Jorn on 09/02/2019.
//

#include "C4Abstract.h"

std::vector<TrappedSlot> C4Abstract::LocateTrapsInState(const State &state, bool filterDoubles)
{
    std::vector<TrappedSlot> traps;
    for(int row = 0; row < state.size(); row++){
        for(int col = 0; col < state[row].size(); col++) {
            Position pos = Position(row, col);
            Player trapper = GetSlotTrappedByPlayer(state, pos);
            if(trapper != Player::None)
            {
                TrappedSlot ts;
                ts.player = trapper;
                ts.position = pos;
                traps.push_back(ts);
            }
        }
    }
    return traps;
}

Player C4Abstract::GetSlotTrappedByPlayer(const State &state, const Position &pos)
{
    Player occupant = state[pos.row][pos.column];
    if(occupant != Player::None) return Player::None; // This slot cannot be trapped as it is not empty.

    bool trapFoundX = false;
    bool trapFoundO = false;

    Player pw = Player::None;
    Player pe = Player::None;

    int cw = 0;
    int ce = 0;

    auto row = state[pos.row];

    // Concurrent coins left
    if(pos.column > 0 && row[pos.column - 1] != Player::None) {
        pw = row[pos.column - 1];
        cw = 1;
        if(pos.column > 1 && row[pos.column - 2] == pw) {
            cw++;
            if(pos.column > 2 && row[pos.column - 3] == pw) cw++;
        }
    }
    if(cw == 3 && pw != Player::None) { // Trap to the west
        if(pw == Player::X) trapFoundX = true;
        else trapFoundO = true;
    }


    // Concurrent coins right
    if(pos.column < 6 && row[pos.column + 1] != Player::None) {
        pe = row[pos.column + 1];
        ce = 1;
        if(pos.column < 5 && row[pos.column + 2] == pe) {
            ce++;
            if(pos.column < 4 && row[pos.column + 3] == pe) ce++;
        }
    }
    if(ce == 3 && pe != Player::None) { // Trap to the east
        if(pe == Player::X) trapFoundX = true;
        else trapFoundO = true;
        if(trapFoundO && trapFoundX) return Player::Both;
    }
    if(pe == pw && ce + cw >= 3) {  // E/W is same player
        if(pe == Player::X) trapFoundX = true;
        else trapFoundO = true;
        if(trapFoundO && trapFoundX) return Player::Both;
    }

    // Diagonal A
    pw = Player::None;
    pe = Player::None;

    cw = 0;
    ce = 0;

    if(pos.column > 0 && pos.row > 0 && state[pos.row - 1][pos.column - 1] != Player::None) {
        pw = state[pos.row - 1][pos.column - 1];
        cw = 1;
        if(pos.column > 1 && pos.row > 1 && state[pos.row - 2][pos.column - 2] == pw) {
            cw++;
            if(pos.column > 2 && pos.row > 2 && state[pos.row - 3][pos.column - 3] == pw) cw++;
        }
    }
    if(cw == 3 && pw != Player::None) { // Trap to the west
        if(pw == Player::X) trapFoundX = true;
        else trapFoundO = true;
        if(trapFoundO && trapFoundX) return Player::Both;
    }

    if(pos.column < 6 && pos.row < 5 && state[pos.row + 1][pos.column + 1] != Player::None) {
        pe = state[pos.row + 1][pos.column + 1];
        ce = 1;
        if(pos.column < 5 && pos.row < 4 && state[pos.row + 2][pos.column + 2] == pe) {
            ce++;
            if(pos.column < 4 && pos.row < 3 && state[pos.row + 3][pos.column + 3] == pe) ce++;
        }
    }
    if(ce == 3 && pe != Player::None) { // Trap to the east
        if(pe == Player::X) trapFoundX = true;
        else trapFoundO = true;
        if(trapFoundO && trapFoundX) return Player::Both;
    }
    if(pe == pw && ce + cw >= 3) {  // E/W is same player
        if(pe == Player::X) trapFoundX = true;
        else trapFoundO = true;
        if(trapFoundO && trapFoundX) return Player::Both;
    }


    // Diagonal B
    pw = Player::None;
    pe = Player::None;

    cw = 0;
    ce = 0;

    if(pos.column > 0 && pos.row < 5 && state[pos.row + 1][pos.column - 1] != Player::None) {
        pw = state[pos.row + 1][pos.column - 1];
        cw = 1;
        if(pos.column > 1 && pos.row < 4 && state[pos.row + 2][pos.column - 2] == pw) {
            cw++;
            if(pos.column > 2 && pos.row < 3 && state[pos.row + 3][pos.column - 3] == pw) cw++;
        }
    }
    if(cw == 3 && pw != Player::None) { // Trap to the west
        if(pw == Player::X) trapFoundX = true;
        else trapFoundO = true;
        if(trapFoundO && trapFoundX) return Player::Both;
    }

    if(pos.column < 6 && pos.row > 0 && state[pos.row - 1][pos.column + 1] != Player::None) {
        pe = state[pos.row + 1][pos.column + 1];
        ce = 1;
        if(pos.column < 5 && pos.row > 1 && state[pos.row - 2][pos.column + 2] == pe) {
            ce++;
            if(pos.column < 4 && pos.row > 2 && state[pos.row - 3][pos.column + 3] == pe) ce++;
        }
    }
    if(ce == 3 && pe != Player::None) { // Trap to the east
        if(pe == Player::X) trapFoundX = true;
        else trapFoundO = true;
        if(trapFoundO && trapFoundX) return Player::Both;
    }
    if(pe == pw && ce + cw >= 3) {  // E/W is same player
        if(pe == Player::X) trapFoundX = true;
        else trapFoundO = true;
        if(trapFoundO && trapFoundX) return Player::Both;
    }

    // Vertical
    if(pos.row <= 2 && state[pos.row + 1][pos.column] != Player::None &&
    state[pos.row + 1][pos.column] == state[pos.row + 2][pos.column] &&
    state[pos.row + 1][pos.column] == state[pos.row + 3][pos.column])
    {
        if(state[pos.row + 1][pos.column] == Player::X) trapFoundX = true;
        else trapFoundO = true;
    }

    if(trapFoundO && trapFoundX) return Player::Both;
    if(trapFoundX) return Player::X;
    if(trapFoundO) return Player::O;
    return Player::None;
}

std::array<int, 7> C4Abstract::GetColumnProgressions(const State &state) {
    std::array<int, 7> prog = {0, 0, 0, 0, 0, 0};
    for(int c = 0; c < 7; c++)
        for(int r = 5; r >= 0; r--)
            if(state[r][c] != Player::None) {
                prog[c] = 6 - r;
            }

    return prog;
}

std::ostream &operator<<(std::ostream& os, const Position &pos) {
    os << "(" << pos.row << ":" << pos.column << ")";
    return os;
}
