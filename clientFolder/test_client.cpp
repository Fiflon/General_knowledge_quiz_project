#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

void sendString(int socket, const std::string &message)
{
    int sizeOfMsg = message.size();
    send(socket, &sizeOfMsg, sizeof(sizeOfMsg), 0);
    send(socket, message.c_str(), sizeOfMsg, 0);
}

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    std::string hello = "hello";

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9867);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection Failed" << std::endl;
        return -1;
    }

    std::string userInput;
    std::cout << "Enter the message to send: ";
    std::getline(std::cin, userInput);
    hello = userInput;
    sendString(sock, hello);
    std::cout << "Hello message sent" << std::endl;

    shutdown(sock, SHUT_RDWR);
    close(sock);
    return 0;
}