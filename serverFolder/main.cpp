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

#define MAX_EVENTS 10
#define PORT 9867

std::unordered_map<int, Player> players;
int active_players = 0;
bool countdown_started = false;
bool game_in_progress = false;
time_t start_time = 0;

bool new_client_connected = false;

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

    if (new_client_connected)
    {
        new_client_connected = false;
        std::cout << "New client connected!" << std::endl;
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}
