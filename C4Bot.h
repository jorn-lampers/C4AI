#ifndef C4BOT_H
#define C4BOT_H

#include <chrono>

#include "C4Game.h"

struct Match {
    State board         = { { { { Player::None } } } };
    int timebank;                   // The time you can exceed a move with before being disqualified; Usually ~10000 ms
    int time_per_move;              // Time per move; Usually 500 ms
    int your_botid;                 // Your bots team; 0 means Player::X, 1 means Player::O
    int round           = 0;        // The round of the match that is being played (every 2 moves = 1 round)
    std::string player_names[2];    // Names of competing Players/Bots
    std::string your_bot;           // The name of your bot?

    std::chrono::time_point<std::chrono::steady_clock> turnStartTime;
    long long int timeElapsedThisTurn() const;

};


class C4Bot {
    Match match;
public:
    void run();
private:
    std::vector<std::string> split(const std::string &s, char delim);
    void move(int timeout);
    void setting(std::string &key, std::string &value);
    void update(std::string &key, std::string &value);


};


#endif
