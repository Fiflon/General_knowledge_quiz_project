#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <unordered_map>
#include <vector>

#include "player.h"
#include "game.h"

int create_server_socket(int port);
int setup_epoll(int server_fd);

int reset_points(std::unordered_map<int, Player> &players);
void handle_new_connection(int epoll_fd, int server_fd, std::unordered_map<int, Player> &players);
int count_active_players(const std::unordered_map<int, Player> &players);
std::vector<std::string> split_string(const char delimiter, const std::string &input, int wordsToFind);

std::string handle_client_message(int client_fd, std::unordered_map<int, Player> &players, std::string &full_message, Game &game);

bool client_disconnected_or_error(int n, int client_fd, std::unordered_map<int, Player> &players, int epoll_fd);

std::string get_parsed_ranking(std::unordered_map<int, Player> &players);
#endif