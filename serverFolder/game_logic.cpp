#include "game_logic.h"
#include <iostream>
#include <sys/socket.h>
#include <string>
#include <unistd.h>
#include <vector>

std::vector<std::string> questions = {
    "Question 1", "Question 2", "Question 3",
    "Question 4", "Question 5", "Question 6",
    "Question 7", "Question 8", "Question 9", "Question 10"};

// bool game_in_progress = false;

void start_game(std::unordered_map<int, Player> &players, bool *game_in_progress)
{
    *game_in_progress = true;
    std::string start_msg = "Game is starting now!\n";
    for (auto &p : players)
    {
        send(p.second.fd, start_msg.c_str(), start_msg.size(), 0);
    }

    for (const auto &question : questions)
    {
        std::string question_msg = "Next question: " + question + "\n";
        for (auto &p : players)
        {
            send(p.second.fd, question_msg.c_str(), question_msg.size(), 0);
        }
        sleep(10);
    }

    std::string end_msg = "Game over! Thanks for playing!\n";
    for (auto &p : players)
    {
        send(p.second.fd, end_msg.c_str(), end_msg.size(), 0);
    }
    *game_in_progress = false;
}
