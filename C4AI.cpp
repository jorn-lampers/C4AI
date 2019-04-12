#include "C4AI.h"

#include <iostream>

#include "TreeSearch.h"
#include "C4Abstract.h"

Move C4AI::FindBestMove(const Match & match)
{
    Move bestMove = -1;

    // Find all moves and rate them
    Player me = getCurrentPlayer(match.board);
    std::vector<Move> moves = getMoves(match.board);

    // Edge cases...
    if(moves.empty()) std::cerr << "ERROR: Board appears to be full, yet AI is asked to pick a move!" << std::endl;
    if(moves.size() == 1) return moves[0]; // Might occur later in matches

    // Rate all moves, safe their scores
    int moveRatings [moves.size()];
    int searchDepth = INITIAL_SEARCH_DEPTH;

    do {
        if(searchDepth > INITIAL_SEARCH_DEPTH) std::cerr << "Enough time left to do another pass with depth: " << searchDepth << "." << std::endl;
        std::cerr << "Starting pass #" << searchDepth - INITIAL_SEARCH_DEPTH + 1 << " with a search depth of " << searchDepth << "." << std::endl;

        bool searchTreeExhausted = true;

        for (int i = 0; i < moves.size(); i++) {
            bool fullMoveTreeEvaluated = true;
            State child = doMove(match.board, moves[i]);
            moveRatings[i] = TreeSearch::MiniMaxAB(child, EvaluateState, GetChildStates, searchDepth, false, me, Score::Should_Lose, Score::Guaranteed_Win, &fullMoveTreeEvaluated);
            if(moveRatings[i] == Score::Guaranteed_Win) {
                std::cerr << "Found a route to a guaranteed win... Breaking off search!" << std::endl;
                return moves[i];
            }
            if(!fullMoveTreeEvaluated) searchTreeExhausted = false;
            else std::cerr << "Exhausted search tree of move #" << i << "." << std::endl;
        }
        std::cerr << "Finished pass #" << searchDepth - INITIAL_SEARCH_DEPTH + 1 << "." << std::endl;
        std::cerr << "Time elapsed: " << match.timeElapsedThisTurn() << "/" << match.time_per_move << " ms." << std::endl;
        if(searchTreeExhausted)
        {
            std::cerr << "Entire search tree was exhausted! Bot knows how this game will end if played perfectly by both sides." << std::endl;
            break;
        } else std::cerr << "MiniMax did not find definite outcome for a perfectly played match..." << std::endl;
        searchDepth++; // Increase search depth for next iteration.
    }
    while ( // Keep searching 1 level deeper if there's enough time left, do not risk loosing time-bank time during first 2 rounds, its not worth it
            (match.timeElapsedThisTurn() * 3 < match.time_per_move && match.timebank > (5 * match.time_per_move) && match.round > 2)
            || // Game has a branching factor of 7, expect the time elapsed each iteration to be multiplied with this factor in worst case.
            (match.timeElapsedThisTurn() * 6 < match.time_per_move )
    );

    // Find the highest score amongst rated moves
    int highestRating = moveRatings[0];
    for(int i = 0; i < moves.size(); i++)
        if (moveRatings[i] > highestRating) highestRating = moveRatings[i];

    if(highestRating == Score::Should_Lose)
        std::cerr << "All examined moves result in a loss! Chances are i will lose." << std::endl;

    // There might be multiple moves with the same -best score, put all of them in a list
    std::vector<Move> bestMoves;
    for(int i = 0; i < moves.size(); i++)
        if(moveRatings[i] == highestRating)
            bestMoves.push_back(moves[i]);

    if(bestMoves.empty()) std::cerr << "ERROR: Best moves list is empty!" << std::endl;
    else if(bestMoves.size() == 1) bestMove = bestMoves[0];
    else {
        bool fullMoveTreeEvaluated = true;
        std::cerr << "Moves yielding equal results have been found, picking one using secondary heuristics: " << std::endl;
        int highest = -1000;
        auto startPass2 = match.timeElapsedThisTurn();
        for (Move m : bestMoves) {
            State moveResult = doMove(match.board, m);
            int score = TreeSearch::MiniMaxAB(moveResult, RateSecondaryHeuristic, GetChildStates, 3, false, me, Score::Min, Score::Max, &fullMoveTreeEvaluated);
            std::cerr << "  - Move " << m << " yields a heuristic score of: " << score << "." << std::endl;
            if (score > highest || bestMove == -1) {
                highest = score;
                bestMove = m;
            }
        }
        auto pass2Time = match.timeElapsedThisTurn() - startPass2;
        std::cerr << "Finished second pass in " << pass2Time << " ms." << std::endl;

    }
    if(bestMove == -1) std::cerr << "ERROR: Best move not found!" << std::endl;
    return bestMove; // Return highest-rating move
}

int C4AI::EvaluateState(const State & state, const Player & positive)
{
    Player winner = getWinner(state);                                       // Is there a winner?
    if(winner == positive) return Score::Guaranteed_Win;                    // Bot has won in evaluated state
    if(winner == Player::None) return RatePrimaryHeuristic(state, positive);// No winner, rate state with heuristics
    return Score::Should_Lose;                                              // Opponent has won in evaluated state
}

int C4AI::RateFinishedGame(const State & state, const Player & positive)
{
    Player winner = getWinner(state);
    if(winner == positive) return Score::Guaranteed_Win;
    if(winner == Player::None) return 0;                                    // Finished game resulted in a tie
    return Score::Should_Lose;
}

std::vector<State> C4AI::GetChildStates(const State &state)
{
    std::vector<State> children;
    std::vector<Move> moves = getMoves(state);
    for(Move m : moves) children.push_back(doMove(state, m));
    return children;
}

int C4AI::RatePrimaryHeuristic(const State &state, const Player &positive)
{
    if(getMoves(state).empty()) return RateFinishedGame(state, positive);
    int trapScore = RateByPotentialTraps(state, positive);
    return trapScore;
}

int C4AI::RateSecondaryHeuristic(const State &state, const Player &positive) {
    if(getMoves(state).empty()) return RateFinishedGame(state, positive);
    else return RateByPotentialFours(state, positive);
}

int C4AI::RateByPotentialFours(const State &state, const Player &positive) {
    int rating = 0; // Rating starts out neutral
    for (int col = 0; col < 7; col++) {
        // Start searching at the bottom row, as coins stack upwards
        for (int row = 5; row >= 0; row--)
        { // For every position on the board:
            Player coin = state[row][col];
            if (coin != Player::None)
            {
                int mod = coin == positive ? Heur_P4_Me : Heur_P4_Opp;
                Player opp = coin == Player::X ? Player::O : Player::X;
                // Horizontal:
                if (col >= 3) // West (col decrement)
                    if(state[row][col-1] != opp && state[row][col-2] != opp && state[row][col-3] != opp) {
                        rating += mod*Heur_P4_Abs_H; // 1 Point for single coin in potential unblocked c4
                        if(state[row][col-1] == coin) rating += mod*Heur_P4_Abs_H; // Found another coin of player in potential c4
                        if(state[row][col-2] == coin) rating += mod*Heur_P4_Abs_H; // Found another coin of player in potential c4
                        if(state[row][col-3] == coin) rating += mod*Heur_P4_Abs_H; // Found another coin of player in potential c4
                    }
                if (col <= 3) // East (col increment)
                    if(state[row][col+1] != opp && state[row][col+2] != opp && state[row][col+3] != opp) {
                        rating += mod*Heur_P4_Abs_H;
                        if(state[row][col+1] == coin) rating += mod*Heur_P4_Abs_H; // Found another coin of player in potential c4
                        if(state[row][col+2] == coin) rating += mod*Heur_P4_Abs_H; // Found another coin of player in potential c4
                        if(state[row][col+3] == coin) rating += mod*Heur_P4_Abs_H; // Found another coin of player in potential c4
                    }
                // Vertical:
                if (row >= 3) { // South (row decrement)
                    if(state[row-1][col] != opp && state[row-2][col] != opp && state[row-3][col] != opp) {
                        rating += mod*Heur_P4_Abs_V;
                        if(state[row-1][col] == coin) rating += mod*Heur_P4_Abs_V; // Found another coin of player in potential c4
                        if(state[row-2][col] == coin) rating += mod*Heur_P4_Abs_V; // Found another coin of player in potential c4
                        if(state[row-3][col] == coin) rating += mod*Heur_P4_Abs_V; // Found another coin of player in potential c4
                    }
                } else if (row <= 2) // North (row increment)
                    if(state[row+1][col] != opp && state[row+2][col] != opp && state[row+3][col] != opp) {
                        rating += mod*Heur_P4_Abs_V;
                        if(state[row+1][col] == coin) rating += mod*Heur_P4_Abs_V; // Found another coin of player in potential c4
                        if(state[row+2][col] == coin) rating += mod*Heur_P4_Abs_V; // Found another coin of player in potential c4
                        if(state[row+3][col] == coin) rating += mod*Heur_P4_Abs_V; // Found another coin of player in potential c4
                    }
                // Diagonal:
                if (row >= 3) {
                    if (col >= 3) // South-West
                        if(state[row-1][col-1] != opp && state[row-2][col-2] != opp && state[row-3][col-3] != opp) {
                            rating += mod*Heur_P4_Abs_D;
                            if(state[row-1][col-1] == coin) rating += mod*Heur_P4_Abs_D; // Found another coin of player in potential c4
                            if(state[row-2][col-2] == coin) rating += mod*Heur_P4_Abs_D; // Found another coin of player in potential c4
                            if(state[row-3][col-3] == coin) rating += mod*Heur_P4_Abs_D; // Found another coin of player in potential c4
                        }
                    if (col <= 3) // South-East
                        if(state[row-1][col+1] != opp && state[row-2][col+2] != opp && state[row-3][col+3] != opp) {
                            rating += mod*Heur_P4_Abs_D;
                            if(state[row-1][col+1] == coin) rating += mod*Heur_P4_Abs_D; // Found another coin of player in potential c4
                            if(state[row-2][col+2] == coin) rating += mod*Heur_P4_Abs_D; // Found another coin of player in potential c4
                            if(state[row-3][col+3] == coin) rating += mod*Heur_P4_Abs_D; // Found another coin of player in potential c4
                        }
                } else if (row <= 2) {
                    if (col >= 3)  // North-West
                        if(state[row+1][col-1] != opp && state[row+2][col-2] != opp && state[row+3][col-3] != opp) {
                            rating += mod*Heur_P4_Abs_D;
                            if(state[row+1][col-1] == coin) rating += mod*Heur_P4_Abs_D; // Found another coin of player in potential c4
                            if(state[row+2][col-2] == coin) rating += mod*Heur_P4_Abs_D; // Found another coin of player in potential c4
                            if(state[row+3][col-3] == coin) rating += mod*Heur_P4_Abs_D; // Found another coin of player in potential c4
                        }
                    if (col <= 3)  // North-East
                        if(state[row+1][col+1] != opp && state[row+2][col+2] != opp && state[row+3][col+3] != opp) {
                            rating += mod*Heur_P4_Abs_D;
                            if(state[row+1][col+1] == coin) rating += mod*Heur_P4_Abs_D; // Found another coin of player in potential c4
                            if(state[row+2][col+1] == coin) rating += mod*Heur_P4_Abs_D; // Found another coin of player in potential c4
                            if(state[row+3][col+1] == coin) rating += mod*Heur_P4_Abs_D; // Found another coin of player in potential c4
                        }
                }
            } else goto topOfRowFound; // There's no more coins in this row, continue outer loop to check next column
        }
        topOfRowFound:;
    }
    return rating;
}

int C4AI::RateByPotentialTraps(const State &state, const Player &positive)
{
    /// First trap in each column gets awarded 1 point,
    /// 2 concurrent traps of the same player in a column gets awarded 3 points
    int score = 0;
    auto traps = C4Abstract::LocateTraps(state);
    auto progression = C4Abstract::GetColumnProgressions(state);

    for(TrappedSlot ts : traps)
    {
        if(ts.player == Player::Both) continue;
        int trapHeight = 6 - ts.position.row;
        int coinsToTrap = trapHeight - progression[ts.position.column];
        int points = (6 - coinsToTrap)*Score::Heur_T_Row_Height_Mod;
        if(ts.player == positive) score += points;
        else score -= points;
    }

    return score;

}
