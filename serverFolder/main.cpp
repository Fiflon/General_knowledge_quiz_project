#include "server_utils.h"
#include "player.h"
#include "nickname_handler.h"
#include "questions.h"
#include "game.h"

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
time_t start_time = 0;

const int countdown_time = 20;

bool new_client_connected = false;

Game game;

int send_string(int socket, const std::string &message)
{
    int sizeOfMsg = message.size();
    int err = send(socket, &sizeOfMsg, sizeof(sizeOfMsg), 0);
    if (err == -1)
    {
        std::cout << "Error sending message size" << std::endl;
        return -1;
    }
    else if (err != sizeof(sizeOfMsg))
    {
        std::cout << "Wrong amount of data send" << std::endl;
        return -2;
    }

    err = send(socket, message.c_str(), sizeOfMsg, 0);
    if (err == -1)
    {
        std::cout << "Error sending message" << std::endl;
        return -1;
    }
    else if (err != sizeOfMsg)
    {
        std::cout << "Wrong amount of message data send" << std::endl;
        return -2;
    }

    return 0;
}

std::string recv_string(int socket, std::unordered_map<int, Player> &players, int epoll_fd, int *active_players)
{
    int sizeOfMsg;
    int n = recv(socket, &sizeOfMsg, sizeof(sizeOfMsg), 0);
    std::string nick = players[socket].nickname;

    if (client_disconnected_or_error(n, socket, players, epoll_fd, active_players))
    {
        if (nick != "")
        {
            return "dis|" + nick + "|";
        }
        else
        {
            return "-100";
        }
    }

    // moze dodac sprawdznie wielkosci

    char buffer[sizeOfMsg + 1];

    n = recv(socket, buffer, sizeOfMsg, 0);

    if (client_disconnected_or_error(n, socket, players, epoll_fd, active_players))
    {
        if (nick != "")
        {
            return "dis|" + nick + "|";
        }
        else
        {
            return "-100";
        }
    }

    if (n != sizeOfMsg)
    {
        std::cerr << "Error: n != sizeOfMsg" << std::endl;
        return "-200";
    }

    buffer[sizeOfMsg] = '\0';
    return std::string(buffer);
}

int send_message_to_all(const std::unordered_map<int, Player> &players, const std::string &message)
{
    std::cout << "Sending message to all players: " << message << std::endl;
    for (const auto &p : players)
    {
        std::cout << "Sending message to player: " << p.second.nickname << std::endl;

        if (send_string(p.second.fd, message) != 0)
        {
            std::cout << "Error sending message to player: " << p.second.nickname << std::endl;
            return -1;
        }
        else
        {
            std::cout << "Message sent to player: " << p.second.nickname << std::endl;
        }
    }
    return 0;
}

int main()
{

    int server_fd = create_server_socket(PORT);
    int epoll_fd = setup_epoll(server_fd);

    struct epoll_event events[MAX_EVENTS];

    while (true)
    {

        if (active_players < 3 && countdown_started)
        {
            countdown_started = false;
            start_time = 0;
            std::cout << "Not enough players to start the game. Waiting for more players..." << std::endl;
            // wyslij gam
            send_message_to_all(players, "gam|1|");
        }

        if (time(0) >= start_time && countdown_started && active_players >= 3)
        {
            send_message_to_all(players, "gam|0|");
            countdown_started = false;
            game.start_game();
            std::string current_ranking = get_parsed_ranking(players);
            send_message_to_all(players, current_ranking);
            game.next_question();
            send_message_to_all(players, game.get_current_question_parsed());
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
                std::string recived_message = recv_string(events[n].data.fd, players, epoll_fd, &active_players);

                if (recived_message == "-100" || recived_message == "-200")
                {
                    continue;
                }

                std::cout << "Full message: " << recived_message << std::endl;

                std::string response = handle_client_message(events[n].data.fd, players, &active_players, recived_message, game);

                if (response.rfind("dis", 0) == 0)
                {
                    send_message_to_all(players, response);
                    continue;
                }

                if (response == "nic|0|" && game.is_game_in_progress())
                {
                    response = "nic|4|";
                }
                else if (response == "nic|0|" && countdown_started)
                {
                    response = "nic|5|";
                }
                std::cout << "Response: " << response << std::endl;

                send_string(events[n].data.fd, response);
            }
        }

        if (active_players < 2 && game.is_game_in_progress())
        {
            std::string current_ranking = get_parsed_ranking(players);
            send_message_to_all(players, current_ranking);
            game.end_game();

            reset_points(players);
            send_message_to_all(players, "gam|4|");
        }

        if (game.is_game_in_progress())
        {
            if (game.get_time_left() > 0)
            {
                continue;
            }

            std::string current_ranking = get_parsed_ranking(players);
            send_message_to_all(players, current_ranking);

            if (game.next_question() == -1)
            {
                std::cout << "Game ended!" << std::endl;
                reset_points(players);

                send_message_to_all(players, "gam|3|");
            }
            else
            {
                send_message_to_all(players, game.get_current_question_parsed());
            }
        }

        if (active_players >= 3 && !countdown_started && !game.is_game_in_progress())
        {
            countdown_started = true;
            std::cout << "Game will start in " << countdown_time << " seconds!" << std::endl;
            send_message_to_all(players, "gam|2|");

            time_t now = time(0);
            start_time = now + countdown_time;
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}
