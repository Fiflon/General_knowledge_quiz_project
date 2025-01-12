#ifndef GAME_H
#define GAME_H

#include "questions.h"
#include "player.h"
#include <unordered_map>
#include <vector>
#include <ctime>

class Game
{
public:
    Game();
    int start_game();
    int end_game();
    int next_question();
    bool is_game_in_progress() const;
    std::string get_current_question_parsed() const;
    std::string get_current_question_answer() const;
    int get_current_question_index() const;
    int get_current_question_number() const;
    int get_question_difficulty() const;
    time_t get_time_left() const;

private:
    bool game_in_progress;
    std::vector<Question> questions;
    int current_question_index;
    int current_question_number;
    time_t question_start_time;
    const time_t question_time_limit = 8;
};

#endif // GAME_H