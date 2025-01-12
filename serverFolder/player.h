#ifndef PLAYER_H
#define PLAYER_H

#include <string>

struct Player
{
    int fd;
    std::string nickname;
    int points;

    time_t deadline_time;
};

#endif
