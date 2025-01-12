#include "server_utils.h"
#include "player.h"
#include "nickname_handler.h"
#include "game.h"

#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <string>
#include <vector>

void set_recv_timeout(int socket, int seconds, int microseconds)
{
    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = microseconds;

    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        perror("setsockopt SO_RCVTIMEO");
    }
}

void set_nonblocking(int socket)
{
    int opts = fcntl(socket, F_GETFL);
    if (opts < 0)
    {
        perror("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    opts |= O_NONBLOCK;
    if (fcntl(socket, F_SETFL, opts) < 0)
    {
        perror("fcntl(F_SETFL)");
        exit(EXIT_FAILURE);
    }
}

void set_blocking(int socket)
{
    int flags = fcntl(socket, F_GETFL, 0);
    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        exit(EXIT_FAILURE);
    }

    flags &= ~O_NONBLOCK;
    if (fcntl(socket, F_SETFL, flags) == -1)
    {
        perror("fcntl F_SETFL");
        exit(EXIT_FAILURE);
    }
}

int create_server_socket(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    const int one = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 100) == -1)
    {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    return server_fd;
}

int setup_epoll(int server_fd)
{
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("epoll_create1");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev = {};
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1)
    {
        perror("epoll_ctl: server_fd");
        close(server_fd);
        close(epoll_fd);
        exit(EXIT_FAILURE);
    }

    return epoll_fd;
}

void handle_new_connection(int epoll_fd, int server_fd, std::unordered_map<int, Player> &players)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd == -1)
    {
        perror("accept");
        return;
    }

    set_recv_timeout(client_fd, 1, 0);

    struct epoll_event ev = {};
    ev.events = EPOLLIN;
    ev.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
    {
        perror("epoll_ctl: client_fd");
        close(client_fd);
        return;
    }

    players[client_fd] = {client_fd, "", 0, (time(0) + 50)};
}

std::vector<std::string> split_string(const char delimiter, const std::string &input, int wordsToFind)
{
    std::vector<std::string> result;
    std::string currentWord;
    int count = 0;

    for (size_t i = 0; i < input.size(); ++i)
    {
        char c = input[i];
        if (c == delimiter)
        {
            if (!currentWord.empty())
            {
                result.push_back(currentWord);
                currentWord.clear();
                count++;

                if (count == wordsToFind)
                {
                    // overflow = input.substr(i + 1); // w razie potrzeby odkomentować
                    // rest_n = input.size() - i - 1; // zastanow sie nad tym czy potrzebujemy overflow trzymac
                    return result;
                }
            }
        }
        else
        {
            currentWord += c;
        }
    }

    if (!currentWord.empty())
    {
        result.push_back(currentWord);
    }

    // rest_n = 0;
    return result;
}
std::string handle_client_message(int client_fd, std::unordered_map<int, Player> &players, int *active_players, std::string &full_message, Game &game)
{
    std::string type = full_message.substr(0, 3);
    std::string response = "-999";

    if (type == "nic")
    {
        std::cout << "Nic" << std::endl;
        std::vector<std::string> words = split_string('|', full_message, 2);

        response = handle_new_client_nickname(client_fd, players, active_players, words[1]);
    }
    else if (type == "ans")
    {
        if (game.is_game_in_progress() == false)
        {
            std::cout << "Game not in progress" << std::endl;
            response = "ans|3|";
        }

        time_t time_left_to_answer = game.get_time_left();
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        int milliseconds = 1000 - (ts.tv_nsec / 1000000);
        std::cout << "Answer" << std::endl;
        std::vector<std::string> words = split_string('|', full_message, 3);
        if (game.get_current_question_number() != std::stoi(words[1]))
        {
            std::cout << "Wrong question number" << std::endl;
            response = "ans|2|";
        }

        else if (game.get_current_question_answer() != words[2])
        {
            std::cout << "Wrong answer" << std::endl;
            response = "ans|1|";
        }

        else
        {
            std::cout << "Correct answer" << std::endl;
            int points_to_add = (time_left_to_answer * 8) + (game.get_question_difficulty() * 4);
            points_to_add += milliseconds / 200;
            std::cout << milliseconds / 200 << std::endl;

            players[client_fd]
                .points += points_to_add;
            std::cout << "Current points: " << players[client_fd].points << players[client_fd].nickname << std::endl;
            response = "ans|0|";
        }
    }
    else if (type == "dis")
    {
        response = full_message;
    }
    else if (type == "exi")
    {

        response = full_message;
    }
    else
    {
        std::cout << "unknown" << std::endl;
    }

    /*     for (const auto &word : words)
        {
            std::cout << "Word: " << word << std::endl;
            send(client_fd, word.c_str(), word.size(), 0);
        }
     */
    return response;
}

bool client_disconnected_or_error(int n, int client_fd, std::unordered_map<int, Player> &players, int epoll_fd, int *active_players)
{
    if (n > 0)
    {
        return false;
    }

    if (n == 0)
    {
        std::cout << "Client disconnected: " << client_fd << std::endl;
    }
    else
    {
        perror("read");
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);

    if (players[client_fd].nickname != "" && players.erase(client_fd) != 0)
    {
        (*active_players)--;
    };

    std::cout << "Active players: " << *active_players << std::endl;
    return true;
}

int delete_player(int client_fd, std::unordered_map<int, Player> &players, int *active_players, int epoll_fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
    shutdown(client_fd, SHUT_RDWR);
    close(client_fd);

    if (players[client_fd].nickname != "" && players.erase(client_fd) != 0)
    {
        (*active_players)--;
    };

    std::cout << "Active players: " << *active_players << std::endl;
    return 0;
}

int reset_points(std::unordered_map<int, Player> &players)
{
    for (auto &p : players)
    {
        p.second.points = 0;
    }
    return 0;
}

std::string get_parsed_ranking(std::unordered_map<int, Player> &players)
{
    std::vector<std::pair<std::string, int>> ranking;
    for (const auto &p : players)
    {
        if (p.second.nickname == "")
        {
            continue;
        }

        ranking.push_back({p.second.nickname, p.second.points});
    }

    std::sort(ranking.begin(), ranking.end(), [](const std::pair<std::string, int> &a, const std::pair<std::string, int> &b)
              { return b.second < a.second; });

    std::string parsed_ranking = "rank|";
    for (const auto &r : ranking)
    {
        parsed_ranking += r.first + ":" + std::to_string(r.second) + "|";
    }

    return parsed_ranking;
}

int delete_inactive_players(std::unordered_map<int, Player> &players, int *active_players, int epoll_fd)
{
    std::vector<int> inactive_players;
    for (const auto &p : players)
    {
        if (time(0) > p.second.deadline_time)
        {

            if (p.second.nickname == "")
            {
                std::cout << "Player not active" << std::endl;
                inactive_players.push_back(p.first);
                continue;
            }
            std::cout << "Deleting inactive player: " << p.second.nickname << std::endl;
            std::cout << time(0) << " > " << p.second.deadline_time << std::endl;

            inactive_players.push_back(p.first);
        }
    }

    for (const auto &p : inactive_players)
    {
        delete_player(p, players, active_players, epoll_fd);
    }

    return 0;
}