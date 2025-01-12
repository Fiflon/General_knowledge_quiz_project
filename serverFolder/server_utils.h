#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <unordered_map>
#include <vector>

#include "player.h"
#include "game.h"
void set_recv_timeout(int socket, int seconds, int microseconds);
void set_blocking(int socket);
void set_nonblocking(int socket);

int create_server_socket(int port);
int setup_epoll(int server_fd);

int reset_points(std::unordered_map<int, Player> &players);
int delete_player(int client_fd, std::unordered_map<int, Player> &players, int *active_players, int epoll_fd);

void handle_new_connection(int epoll_fd, int server_fd, std::unordered_map<int, Player> &players);
std::vector<std::string> split_string(const char delimiter, const std::string &input, int wordsToFind);

std::string handle_client_message(int client_fd, std::unordered_map<int, Player> &players, int *active_players, std::string &full_message, Game &game);

bool client_disconnected_or_error(int n, int client_fd, std::unordered_map<int, Player> &players, int epoll_fd, int *active_players);

std::string get_parsed_ranking(std::unordered_map<int, Player> &players);
#endif