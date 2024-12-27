#ifndef QUESTIONS_H
#define QUESTIONS_H

#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_set>

struct Question
{
    int id;
    std::string content;
    std::string ans_a;
    std::string ans_b;
    std::string ans_c;
    std::string ans_d;
    char correct_ans;
    int difficulty;
};

std::vector<Question> pick_questions();

#endif
