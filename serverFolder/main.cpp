#include "server_utils.h"
#include "player.h"
#include "game_logic.h"
#include "nickname_handler.h"
#include "questions.h"

#include <iostream>
#include <unordered_map>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <locale>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_EVENTS 10
#define PORT 9867

std::unordered_map<int, Player> players;
int active_players = 0;
bool countdown_started = false;
bool game_in_progress = false;
time_t start_time = 0;

bool new_client_connected = false;

void sendString(int socket, const std::string &message)
{
    int sizeOfMsg = message.size();
    send(socket, &sizeOfMsg, sizeof(sizeOfMsg), 0);
    send(socket, message.c_str(), sizeOfMsg, 0);
}

std::string recvString(int socket, std::unordered_map<int, Player> &players, int epoll_fd, int *active_players)
{
    int sizeOfMsg;
    std::cout << "1" << std::endl;
    int n = recv(socket, &sizeOfMsg, sizeof(sizeOfMsg), 0);
    if (n <= 0)
    {
        if (client_disconnected_or_error(n, socket, players, epoll_fd, active_players))
        {
            return "-100";
        }
        std::cerr << "Error receiving size of message" << std::endl;
        return "";
    }

    if (sizeOfMsg <= 0 || sizeOfMsg > 1024) // Dodajemy dodatkowe sprawdzenie rozmiaru wiadomości
    {
        std::cerr << "Invalid message size: " << sizeOfMsg << std::endl;
        return "";
    }

    char buffer[sizeOfMsg + 1];
    std::cout << "2" << std::endl;

    n = recv(socket, buffer, sizeOfMsg, 0);
    if (n <= 0)
    {
        if (client_disconnected_or_error(n, socket, players, epoll_fd, active_players))
        {
            return "-100";
        }
        std::cerr << "Error receiving message" << std::endl;
        return "";
    }

    if (n != sizeOfMsg)
    {
        std::cerr << "Error: n != sizeOfMsg" << std::endl;
        return "";
    }

    buffer[sizeOfMsg] = '\0';
    return std::string(buffer);
}

int main()
{

    int server_fd = create_server_socket(PORT);
    int epoll_fd = setup_epoll(server_fd);

    std::string rest_buffer = "";
    int rest_n = 0;

    struct epoll_event events[MAX_EVENTS];

    while (true)
    {
        if (active_players < 3 && countdown_started)
        {
            countdown_started = false;
            start_time = 0;
            std::cout << "Not enough players to start the game. Waiting for more players..." << std::endl;
        }

        if (time(0) >= start_time && countdown_started && active_players >= 3)
        {
            countdown_started = false;
            game_in_progress = true;
            std::cout << "Game is starting now!" << std::endl;
            // start_game(players);
        }

        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 500);
        if (nfds == -1)
        {
            perror("epoll_wait");
            close(server_fd);
            close(epoll_fd);
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n)
        {
            if (events[n].data.fd == server_fd)
            {
                handle_new_connection(epoll_fd, server_fd, players);
                new_client_connected = true;
            }
            else
            {
                /*                 char buffer[1024] = {0};

                                int bytes_read = read(events[n].data.fd, buffer, sizeof(buffer) - 1);

                                if (client_disconnected_or_error(bytes_read, events[n].data.fd, players, epoll_fd, &active_players))
                                {
                                    continue;
                                }

                                buffer[bytes_read] = '\0';
                 */
                std::string recived_message = recvString(events[n].data.fd, players, epoll_fd, &active_players);
                // std::string full_message = std::string(rest_buffer, rest_n) + std::string(buffer, bytes_read);

                std::cout << "Full message: " << recived_message << std::endl;

                std::string response = handle_client_message(events[n].data.fd, players, &active_players, rest_buffer, rest_n, recived_message);

                // std::cout << rest_buffer << std::endl;

                /*                 if (players.find(events[n].data.fd) != players.end())
                {
                    if (players[events[n].data.fd].nickname.empty())
                    {
                        players[events[n].data.fd].nickname = handle_new_client_nickname(events[n].data.fd, players, &active_players, epoll_fd);
                        std::vector<Question> questions = pick_questions();
                        std::cout << questions[0].content << std::endl;
                    }
                    else
                    {
                        handle_client_message(epoll_fd, events[n].data.fd, players, &active_players);
                    }
                } */
            }
        }
        if (active_players >= 3 && !countdown_started && !game_in_progress)
        {
            countdown_started = true;
            std::cout << "Game will start in 20 seconds!" << std::endl;
            time_t now = time(0);
            start_time = now + 20;
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}
