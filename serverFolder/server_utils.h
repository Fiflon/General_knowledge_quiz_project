#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <unordered_map>
#include "player.h"

int create_server_socket(int port);
int setup_epoll(int server_fd);
void handle_new_connection(int epoll_fd, int server_fd, std::unordered_map<int, Player> &players);
void handle_client_message(int epoll_fd, int client_fd, std::unordered_map<int, Player> &players, int *active_players);

// std::string handle_client_message(int epoll_fd, int client_fd, std::unordered_map<int, Player> &players, int *active_players);

bool client_disconnected_or_error(int n, int client_fd, std::unordered_map<int, Player> &players, int epoll_fd, int *active_players);
#endif