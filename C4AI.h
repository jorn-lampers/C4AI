
#ifndef C4AI_H
#define C4AI_H

#include "C4Bot.h"

/// This class defines some Heuristic functions to analyse game-states in connect4.
/// These functions may be used alongside some search algorithm when winning states
/// can't be found yet due to the games branching factor
const static int INITIAL_SEARCH_DEPTH = 6;

class C4AI {
    enum Score {
        Min = -999999, Max = 999999,
        Neutral = 0, Guaranteed_Win = 1000, Should_Lose = -1000,
        Heur_P4_Me = 1, Heur_P4_Opp = -1, Heur_P4_Abs_V = 1, Heur_P4_Abs_H = 2, Heur_P4_Abs_D = 2,
        Heur_T_Row_Height_Mod = 1
    };

public:
    /// C4AI will return the move it expects to be optimal for the player ...
    /// that's supposed to make a move according to passed Match state object.
    static Move FindBestMove(const Match & state);

    /// Evaluates a state, if a Guaranteed win isn't found it will return ...
    /// the passed states Heuristic score according to 'RateTotalHeuristic'.
    static int EvaluateState(const State & state, const Player & positive);

    /// AI's main heuristic function, this function has a relatively high cost ...
    /// and should not be ran unnecessarily. (ie. on finished games)
    static int RatePrimaryHeuristic(const State &state, const Player &positive);

    /// AI's secondary heuristic function, this function has a relatively high cost ...
    /// and should only be used to break ties between moves yielding equal results ...
    /// while evaluated by the primary heuristic function.
    static int RateSecondaryHeuristic(const State &state, const Player &positive);

    /// Rates board by amount of coins that can still be connected to a win.
    static int RateByPotentialFours(const State &state, const Player &positive);

    /// Rates board semi-recursively by finding traps of 3 coins that can ...
    /// turn into 4 when a coin is dropped under them.
    static int RateByPotentialTraps(const State &state, const Player &positive);

    /// Gets all states that may result from the passed state after a single move
    static std::vector<State> GetChildStates(const State & state);

    static int RateFinishedGame(const State & state, const Player & positive);

};

#endif
