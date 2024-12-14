#include "server_utils.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

void set_nonblocking(int sockfd)
{ // na 99 procent nie bedziemy uzywac
    int opts = fcntl(sockfd, F_GETFL);
    if (opts < 0)
    {
        perror("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    opts |= O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, opts) < 0)
    {
        perror("fcntl(F_SETFL)");
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

    // set_nonblocking(server_fd);

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
    // set_nonblocking(client_fd);

    struct epoll_event ev = {};
    ev.events = EPOLLIN;
    ev.data.fd = client_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
    {
        perror("epoll_ctl: client_fd");
        close(client_fd);
        return;
    }

    players[client_fd] = {client_fd, "", 0};
    std::string welcome_msg = "Welcome! Please enter your nickname:\n";
    send(client_fd, welcome_msg.c_str(), welcome_msg.size(), 0);
}

void handle_client_message(int epoll_fd, int client_fd, std::unordered_map<int, Player> &players, int *active_players)
{
    char buffer[512];
    int n = read(client_fd, buffer, sizeof(buffer) - 1);
    if (client_disconnected_or_error(n, client_fd, players, epoll_fd, active_players))
    {
        return;
    }

    buffer[n] = '\0';
    std::string message(buffer);
    Player &player = players[client_fd];

    // Obsługa wiadomości gracza
    std::cout << "Message from " << player.nickname << ": " << message << std::endl;
    std::string response = "Server received: " + message;
    send(client_fd, response.c_str(), response.size(), 0);
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
    close(client_fd);
    shutdown(client_fd, SHUT_RDWR);

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
    players.erase(client_fd);
    (*active_players)--;
    std::cout << "Active players: " << *active_players << std::endl;
    return true;
}

bool client_disconnected_or_error(int n, int client_fd, std::unordered_map<int, Player> &players, int epoll_fd)
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
    close(client_fd);
    shutdown(client_fd, SHUT_RDWR);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, nullptr);
    players.erase(client_fd);

    return true;
}