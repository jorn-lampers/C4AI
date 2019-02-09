
#include "C4AI.h"
#include "TreeSearch.h"

Move C4AI::FindBestMove(const Match & match)
{
    Move bestMove = -1;

    // Find all moves and rate them
    Player me = getCurrentPlayer(match.board);
    std::vector<Move> moves = getMoves(match.board);

    // Edge cases...
    if(moves.size() == 0) std::cerr << "ERROR: Board appears to be full, yet AI is asked to pick a move!" << std::endl;
    if(moves.size() == 1) return moves[0]; // Might occur later in matches

    // Rate all moves, safe their scores
    int moveRatings [moves.size()];
    int searchDepth = INITIAL_SEARCH_DEPTH;

    do {
        if(searchDepth > INITIAL_SEARCH_DEPTH) std::cerr << "Enough time left to do another pass with depth: " << searchDepth << "." << std::endl;
        std::cerr << "Starting pass #" << searchDepth - INITIAL_SEARCH_DEPTH + 1 << " with a search depth of " << searchDepth << "." << std::endl;

        for (int i = 0; i < moves.size(); i++) {
            State child = doMove(match.board, moves[i]);
            moveRatings[i] = TreeSearch::MiniMaxAB(child, EvaluateState, GetChildStates, searchDepth, false, me, Score::Min, Score::Max);
            if(moveRatings[i] == Score::Guaranteed_Win) {
                std::cerr << "Found a route to a guaranteed win... Breaking off search!" << std::endl;
                return moves[i];
            }
        }
        std::cerr << "Finished pass #" << searchDepth - INITIAL_SEARCH_DEPTH + 1 << "." << std::endl;
        std::cerr << "Time elapsed: " << match.timeElapsedThisTurn() << "/" << match.time_per_move << " ms." << std::endl;
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

    if(bestMoves.size() == 0) std::cerr << "ERROR: Best moves list is empty!" << std::endl;
    else if(bestMoves.size() == 1) bestMove = bestMoves[0];
    else {
        std::cerr << "Moves yielding equal results have been found, picking one using trapping heuristics: " << std::endl;
        int highest;
        int startPass2 = match.timeElapsedThisTurn();
        for (Move m : bestMoves) {
            State moveResult = doMove(match.board, m);
            int score = TreeSearch::MiniMaxAB(moveResult, RateByPotentialTraps, GetChildStates, 7, false, me, Score::Min, Score::Max);
            std::cerr << "  - Move " << m << " yields a heuristic score of: " << score << "." << std::endl;
            if (score > highest || bestMove == -1) {
                highest = score;
                bestMove = m;
            }
        }
        int pass2Time = match.timeElapsedThisTurn() - startPass2;
        std::cerr << "Finished second pass in " << pass2Time << " ms." << std::endl;

    }
    if(bestMove == -1) std::cerr << "ERROR: Best move not found!" << std::endl;
    return bestMove; // Return highest-rating move
}

int C4AI::EvaluateState(const State & state, const Player & positive)
{
    Player winner = getWinner(state);                                       // Is there a winner?
    if(winner == positive) return Score::Guaranteed_Win;                    // Bot has won in evaluated state
    if(winner == Player::None) return RateTotalHeuristic(state, positive);  // No winner, rate state with heuristics
    return Score::Should_Lose;                                              // Opponent has won in evaluated state
}

std::vector<State> C4AI::GetChildStates(const State &state)
{
    std::vector<State> children;
    std::vector<Move> moves = getMoves(state);
    for(Move m : moves) children.push_back(doMove(state, m));
    return children;
}

int C4AI::RateTotalHeuristic(const State &state, const Player &positive)
{
    if(getMoves(state).empty()) return 0;
    int score = RateByPotentialFours(state, positive);
    return score;
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
                if (row >= 3) // South (row decrement)
                    if(state[row-1][col] != opp && state[row-2][col] != opp && state[row-3][col] != opp) {
                        rating += mod*Heur_P4_Abs_V;
                        if(state[row-1][col] == coin) rating += mod*Heur_P4_Abs_V; // Found another coin of player in potential c4
                        if(state[row-2][col] == coin) rating += mod*Heur_P4_Abs_V; // Found another coin of player in potential c4
                        if(state[row-3][col] == coin) rating += mod*Heur_P4_Abs_V; // Found another coin of player in potential c4
                    }
                else if (row <= 2) // North (row increment)
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
    int traps3Positive = 0, traps3Negative = 0;
    Player currentPlayer = getCurrentPlayer(state);
    Player negative = positive == Player::X ? Player::O : Player::X;

    // Check for imminent win (next state)
    for(State s:GetChildStates(state))
        if(getWinner(s) == currentPlayer)
            return (currentPlayer == positive ? Score::Guaranteed_Win : Score::Should_Lose);

    // Store amount of coins in their corresponding columns
    std::array<int, 7> colFirstEmpty = {5, 5, 5, 5, 5, 5, 5}; // The 'first' index of the column that's free
    for(int col = 0; col < 7; col++)
        for(int h = 5; h >= 0; h--)
            if(state[h][col] == Player::None) break;
            else colFirstEmpty[col] = h-1; // A ful col will have a value of -1


    // Real search starts here:
    int posImminentTrapCol = -1, negImminentTrapCol = -1; // If this isn't -1 and another forced reaction is found, current player will lose

    int posPlayerCount = 0;
    int negPlayerCount = 0;

    // Horizontal traps
    for(int x = 0; x < 4; x++)  // x = 0tm3 -> 3tm6
        for(int y = 5; y > colFirstEmpty[x]; y--)
        {
            // Check every horizontal potential trap-group of 4 on current board:
            posPlayerCount = 0;
            negPlayerCount = 0;
            int tc = -1;
            for(int dx = 0; dx < 4; dx++) {
                if (state[y][x + dx] == positive) posPlayerCount++;
                else if (state[y][x + dx] == negative) negPlayerCount++;
                else tc = dx + x;
            }
            // Did we find anything interesting? (Horizontally aligned traps for either player)
            if(posPlayerCount == 3 && negPlayerCount == 0) // Found trap (player)
            {
                // Find difference in height to be made for trap to become a win or a forced move
                int trapColHeight = colFirstEmpty[tc]; // Current height of trap col
                int trapDistance = trapColHeight - y;
                if(trapDistance == 1) { // Trap can be blocked/activated in current state (depending on current player)
                    if (currentPlayer == positive) return Score::Guaranteed_Win; // Winning move can be made in passed state
                    else if (posImminentTrapCol != -1 && posImminentTrapCol != tc) return Score::Guaranteed_Win; // Opponent is in a split, and forced to lose
                    else posImminentTrapCol = tc; // Opponent has to react to this trap if he cannot win this turn TODO: Inspect deeper?
                }
                traps3Positive+=7-trapDistance; // Found a positive trap
            }
            else if(negPlayerCount == 3 && posPlayerCount == 0) // Found trap (opponent)
            {
                // Find difference in height to be made for trap to become a win or a forced move
                int trapColHeight = colFirstEmpty[tc]; // Current height of trap col
                int trapDistance = trapColHeight - y;
                if(trapDistance == 1) { // Trap can be blocked/activated in current state (depending on current player)
                    if (currentPlayer != positive) return Score::Should_Lose; // Winning move can be made by opponent in passed state
                    else if (negImminentTrapCol != -1 && negImminentTrapCol != tc) return Score::Should_Lose; // Opponent is in a split, and forced to lose
                    else negImminentTrapCol = tc; // Opponent has to react to this trap if he cannot win this turn TODO: Inspect deeper?
                }
                traps3Negative+=7-trapDistance; // Found a positive trap
            }
        }

    // Diagonal traps
    for(int x = 0; x < 4; x++)
        for(int y = 0; y < 6; y++)
        {
            for (int dir = -1; dir <= 1; dir += 2)  // Check the two diagonal directions
            {
                if(dir == -1 && y < 3) continue; // Less than four slots in direction were checking
                if(dir == +1 && y > 2) continue; // Less than four slots in direction were checking

                posPlayerCount = 0;
                negPlayerCount = 0;
                int tc = -1; // The column where a trap could reside
                int tr = -1; // The row ''
                for (int d = 0; d < 4; d++)
                {
                    if(state[y+d*dir][x+d] == positive) posPlayerCount++;
                    else if(state[y+d*dir][x+d] == negative) negPlayerCount++;
                    else {
                        tc = d+x; // Empty, could be trap col
                        tr = y+d*dir;
                    }
                }

                // Did we find anything interesting? (Diagonal traps for either player)
                if (posPlayerCount == 3 && negPlayerCount == 0) // Found trap (player)
                {
                    // Find difference in height to be made for trap to become a win or a forced move
                    int trapColHeight = colFirstEmpty[tc]; // Current height of trap col
                    int trapDistance = trapColHeight - tr; // currentHeight - trap row
                    if (trapDistance == 1) { // Trap can be blocked/activated in current state (depending on current player)
                        if (currentPlayer == positive) return Score::Guaranteed_Win; // Winning move can be made in passed state
                        else if (posImminentTrapCol != -1 && posImminentTrapCol != tc) return Score::Guaranteed_Win; // Opponent is in a split, and forced to lose
                        else posImminentTrapCol = tc; // Opponent has to react to this trap if he cannot win this turn TODO: Inspect deeper?
                    }
                    traps3Positive+=7-trapDistance;
                } else if (negPlayerCount == 3 && posPlayerCount == 0) // Found trap (opponent)
                {
                    // Find difference in height to be made for trap to become a win or a forced move
                    int trapColHeight = colFirstEmpty[tc]; // Current height of trap col
                    int trapDistance = trapColHeight - tr; // currentHeight - trap row
                    if (trapDistance == 1) { // Trap can be blocked/activated in current state (depending on current player)
                        if (currentPlayer != positive) return Score::Should_Lose; // Winning move can be made by opponent in passed state
                        else if (negImminentTrapCol != -1 && negImminentTrapCol != tc) return Score::Should_Lose; // Opponent is in a split, and forced to lose
                        else negImminentTrapCol = tc; // Opponent has to react to this trap if he cannot win this turn TODO: Inspect deeper?
                    }
                    traps3Negative+=7-trapDistance;
                }
            }
        }

    // "Vertical traps"
    for(int col = 0; col < 7; col++) {
        if(state[col][0] != Player::None) continue; // This column is full
        for(int row = 0; row < 6-3; row++) {
            if(state[row][col] != Player::None) {
                if(state[row][col] == state[row+1][col] && state[row][col] == state[row+2][col])
                    if(state[row][col] == negative) {
                        traps3Negative+=6;
                    }
                    else traps3Positive+=6;
            } else continue;
        }
    }

    return traps3Positive - traps3Negative; // TODO: traps shouldn't all be of equal worth
}
