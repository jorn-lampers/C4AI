#include "C4Bot.h"
#include "C4AI.h"

#include <iostream>
#include <cmath>
#include <sstream>

void C4Bot::move(int timeout) {
    match.turnStartTime = std::chrono::steady_clock::now();

    C4AI::RateByPotentialTraps(match.board, getCurrentPlayer(match.board));

    std::cerr << "---------------------------------------------------------------------------------------" << std::endl;
    std::cerr << "STARTING MOVE-SEARCH FOR ROUND #" << match.round << " as Player " << getCurrentPlayer(match.board) << "." << std::endl;
    std::cerr << "---------------------------------------------------------------------------------------" << std::endl;

    Move m = C4AI::FindBestMove(match);
    int ms = match.timeElapsedThisTurn();

    std::cerr << "______________________________________________________________________________________________" << std::endl;
    std::cerr << "Search for move finished in " << ms << " milliseconds." << std::endl;
    std::cerr << "Awaiting next turn..." << std::endl;
    std::cerr << "______________________________________________________________________________________________" << std::endl << std::endl;

    std::cout << "place_disc " << m << std::endl;
}

void C4Bot::run()
{
    std::string line;
    while (std::getline(std::cin, line))
    {
        std::vector<std::string> command = split(line, ' ');
        if (command[0] == "settings") setting(command[1], command[2]);
        else if (command[0] == "update" && command[1] == "game") update(command[2], command[3]);
        else if (command[0] == "action" && command[1] == "move") move(std::stoi(command[2]));
        else std::cerr << "Unknown command: " << line << std::endl;
    }
}

void C4Bot::update(std::string &key, std::string &value)
{
    if (key == "round") match.round = std::stoi(value);
    else if (key == "field") {
        int row = 0;
        int col = 0;
        std::vector<std::string> fields = split(value, ',');
        for (std::string &field : fields) {
            if (field == "0") match.board[row][col] = Player::X;
            else if (field == "1") match.board[row][col] = Player::O;
            else match.board[row][col] = Player::None;

            col++;
            if (col == 7) {
                row++;
                col = 0;
            }
        }
    }
}

void C4Bot::setting(std::string &key, std::string &value)
{
    if (key == "timebank")              match.timebank = std::stoi(value);
    else if (key == "time_per_move")    match.time_per_move = std::stoi(value);
    else if (key == "your_bot")         match.your_bot = value;
    else if (key == "field_columns")    match.field_columns = std::stoi(value);
    else if (key == "field_rows")       match.field_rows = std::stoi(value);
    else if (key == "your_botid") {
        match.your_botid = std::stoi(value);
        match.botPlayer = match.your_botid == 0 ? Player::X : Player::O;
    }
    else if (key == "player_names") {
        std::vector<std::string> names = split(value, ',');
        match.player_names[0] = names[0];
        match.player_names[1] = names[1];
    }
}

std::vector<std::string> C4Bot::split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) elems.push_back(item);
    return elems;
}

long long int Match::timeElapsedThisTurn() const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - turnStartTime
    ).count();
}
