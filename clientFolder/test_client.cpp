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

std::string recvString(int socket)
{
    std::cout << "Receiving message" << std::endl;
    int sizeOfMsg;
    int n = recv(socket, &sizeOfMsg, sizeof(sizeOfMsg), 0);
    std::cout << "Receiving message2" << std::endl;
    std::cout << n << std::endl;

    // moze dodac sprawdznie wielkosci

    char buffer[sizeOfMsg + 1];
    std::cout << "Receiving message3" << std::endl;

    n = recv(socket, buffer, sizeOfMsg, 0);

    std::cout << "Receiving message4" << std::endl;
    if (n != sizeOfMsg)
    {
        std::cerr << "Error: n != sizeOfMsg" << std::endl;
        return "-100";
    }

    buffer[sizeOfMsg] = '\0';
    return std::string(buffer);
}

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    time_t now = time(0);
    tm *ltm = localtime(&now);
    int seconds = ltm->tm_sec;
    std::string hello = "nic|test" + std::to_string(seconds) + "|";

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

    sendString(sock, hello);
    while (true)
    {

        std::string response = recvString(sock);

        std::cout << "Server response: " << response << std::endl;
    }

    /*     while (1)
        {
            std::string userInput;
            std::cout << "Enter the message to send: ";
            std::getline(std::cin, userInput);
            hello = userInput;
            sendString(sock, hello);
            std::cout << "message sent" << std::endl;
        } */

    shutdown(sock, SHUT_RDWR);
    close(sock);
    return 0;
}