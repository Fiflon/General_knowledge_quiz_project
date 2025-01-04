#include "game.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

Game::Game() : game_in_progress(false), current_question_index(0), current_question_number(0), question_start_time(0) {}

int Game::start_game()
{
    game_in_progress = true;
    questions = pick_questions();
    current_question_index = -1;
    current_question_number = 0;
    question_start_time = time(0);

    return current_question_number;
}

int Game::end_game()
{
    game_in_progress = false;
    current_question_number = 0;
    current_question_index = 0;

    return 0;
}

int Game::next_question()
{
    if (current_question_index + 1 < (int)questions.size())
    {
        question_start_time = time(0);
        current_question_index++;
        current_question_number++;
        return current_question_number;
    }
    else
    {
        end_game();
        return -1;
    }
}

bool Game::is_game_in_progress() const
{
    return game_in_progress;
}

std::string Game::get_current_question_parsed() const
{
    std::string parsed_question = "que|" + std::to_string(current_question_number) + "|" + questions[current_question_index].content + "|" + questions[current_question_index].ans_a + "|" + questions[current_question_index].ans_b + "|" + questions[current_question_index].ans_c + "|" + questions[current_question_index].ans_d + "|" + std::to_string(questions[current_question_index].difficulty) + "|";
    if (current_question_index < (int)questions.size())
    {
        return parsed_question;
    }
    return "";
}

std::string Game::get_current_question_answer() const
{
    return std::string(1, questions[current_question_index].correct_ans);
}

time_t Game::get_time_left() const
{
    return question_time_limit - (time(0) - question_start_time);
}

int Game::get_current_question_number()
{
    return current_question_number;
}