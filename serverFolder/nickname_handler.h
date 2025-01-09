// nickname_handler.h
#ifndef NICKNAME_HANDLER_H
#define NICKNAME_HANDLER_H

#include <string>
#include <unordered_map>
#include "player.h"

std::string handle_new_client_nickname(int client_fd, std::unordered_map<int, Player> &players, int *active_players, std::string nickname);

#endif // NICKNAME_HANDLER_H
