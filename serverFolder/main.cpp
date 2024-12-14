#include "server_utils.h"
#include "player.h"
#include "game_logic.h"
#include "nickname_handler.h"
#include <iostream>
#include <unordered_map>
#include <vector>
#include <sys/epoll.h>
#include <unistd.h>

#define MAX_EVENTS 10
#define PORT 9867

std::unordered_map<int, Player> players;
int active_players = 0;

int main()
{

    int server_fd = create_server_socket(PORT);
    int epoll_fd = setup_epoll(server_fd);

    struct epoll_event events[MAX_EVENTS];

    while (true)
    {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
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
            }
            else
            {
                if (players[events[n].data.fd].nickname.empty())
                {
                    players[events[n].data.fd].nickname = handle_new_client_nickname(events[n].data.fd, players, &active_players, epoll_fd);
                }
                else
                {

                    handle_client_message(epoll_fd, events[n].data.fd, players, &active_players);
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
    return 0;
}
