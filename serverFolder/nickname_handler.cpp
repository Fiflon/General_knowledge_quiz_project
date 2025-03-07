#include "nickname_handler.h"
#include "server_utils.h"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <sys/socket.h>

bool is_nickname_unique(const std::string &nickname, const std::unordered_map<int, Player> &players)
{
    for (const auto &p : players)
    {
        if (p.second.nickname == nickname)
        {
            return false;
        }
    }
    return true;
}

std::string handle_new_client_nickname(int client_fd, std::unordered_map<int, Player> &players, int *active_players, std::string nickname)
{

    nickname.erase(std::remove(nickname.begin(), nickname.end(), '\n'), nickname.end());

    if (!is_nickname_unique(nickname, players))
    {

        return "nic|1|";
    }
    if (nickname.size() > 20)
    {

        return "nic|2|";
    }
    if (!players[client_fd].nickname.empty())
    {
        return "nic|3|";
    }

    players[client_fd].nickname = nickname;
    (*active_players)++;
    std::cout << "New player connected: " << nickname << std::endl;
    std::cout << "Active players: " << *active_players << std::endl;
    return "nic|0|";
}
