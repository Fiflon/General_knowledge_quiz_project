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

std::string handle_new_client_nickname(int client_fd, std::unordered_map<int, Player> &players, int *active_players, int epoll_fd)
{
    char buffer[512];
    int n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (client_disconnected_or_error(n, client_fd, players, epoll_fd))
    {
        return "";
    }

    buffer[n] = '\0';
    std::string nickname(buffer);
    nickname.erase(std::remove(nickname.begin(), nickname.end(), '\n'), nickname.end());

    if (!is_nickname_unique(nickname, players))
    {
        std::string response = "Nickname already taken. Please enter a different nickname:\n";
        send(client_fd, response.c_str(), response.size(), 0);
        return "";
    }
    (*active_players)++;
    std::cout << "New player connected: " << nickname << std::endl;
    std::cout << "Active players: " << *active_players << std::endl;
    return nickname;
}
