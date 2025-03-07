#include "questions.h"
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_set>

Question questions[] = {
    {1,
     "How many moons does Mars have?",
     "2",
     "3",
     "1",
     "0",
     'a',
     2},
    {2,
     "What is the chemical designation of iron in the periodic table??",
     "Au",
     "I",
     "Fe",
     "Ir",
     'c',
     1},
    {3,
     "Who painted the famous painting 'The Scream'?",
     "Claude Monet",
     "Edvard Munch",
     "Francisco Goya",
     "Vincent van Gogh",
     'b',
     3},
    {4,
     "What is the formula for the area of a circle?",
     "πr²",
     "π²r",
     "πr",
     "2πr",
     'a',
     1},
    {5,
     "what is the name of the company founded by Steve Wozniak, Ronald Wayne and Steve Jobs?",
     "Microsoft",
     "Samsung",
     "Apple",
     "Lenovo",
     'c',
     1},
    {6,
     "What was the name of the leader of the October Revolution in 1917?",
     "Nicholas II Romanov",
     "Vladimir Lenin",
     "Josef Stalin",
     "Pyotr Krasnov",
     'b',
     2},
    {7,
     "What is the name of Africa's highest peak?",
     "Mount Kenya",
     "Kazbek",
     "Kilimanjaro",
     "Uluru",
     'c',
     1},
    {8,
     "Who is the author of the work 'Mathematical Principles of Natural Philosophy'?",
     "Galileo Galilei",
     "Isaac Newton",
     "Niclaus Copernicus",
     "Johannes Kepler",
     'b',
     3},
    {9,
     "What is the capital of Canada?",
     "Ottawa",
     "Toronto",
     "Montreal",
     "Vancouver",
     'a',
     2},
    {10,
     "What was the name of the first man to make a space flight?",
     "Lajka Sobaka",
     "Neil Armstrong",
     "Buzz Aldrin",
     "Yuri Gagarin",
     'd',
     1},
    {11,
     "Who is the author of the famous work 'Don Quixote'?",
     "Antonio Machado",
     "Hermann Hesse",
     "Miguel de Cervantes",
     "C.S. Lewis",
     'c',
     3},
    {12,
     "What distance do you need to run in an Olympic marathon?",
     "43km 205m",
     "42km 195m",
     "42km 205m",
     "43km 195m",
     'b',
     2},
    {13,
     "What is the basic unit of speed in the SI system?",
     "m/s",
     "km/s",
     "km/h",
     "m/h",
     'a',
     2},
    {14,
     "In which country was the compass invented?",
     "Morocco",
     "France",
     "Ethiopia",
     "China",
     'd',
     3},
    {15,
     "What does NATO stand for?",
     "North Atlantic Treaty Organization",
     "National Armed Troops Organization",
     "New Atlantic Trade Operation",
     "Northern Alliance Troops Organization",
     'a',
     2},
    {16,
     "If a solution has a pH of 11 we say it has what reaction?",
     "Neutral",
     "Acidic",
     "Alkaline",
     "Normal",
     'c',
     3},
    {17,
     "What is the name of the Ukrainian who broke the world record in pole vault 17 times?",
     "Anatoly Samoilenko",
     "Sergey Bubka",
     "Volodymyr Marchenko",
     "Anatoly Fomenko",
     'b',
     2},
    {18,
     "Adolf and Rudolf Dassler founded two companies, one was called Adidas and the other?",
     "Birkenstock",
     "Patagonia",
     "Puma",
     "Nike",
     'a',
     3},
    {19,
     "How many planets are there in the solar system?",
     "7",
     "8",
     "9",
     "10",
     'b',
     1},
    {20,
     "What is the name of Rome's first emperor?",
     "Trajan",
     "Marcus Aurelius",
     "Caligula",
     "Octavian Augustus",
     'd',
     3}};

std::random_device rd;
std::mt19937 g(rd());

std::vector<Question> pick_questions()
{
    std::vector<Question> selected_questions;
    std::unordered_set<int> selected_indices;
    std::uniform_int_distribution<> dis(0, 19);

    while (selected_indices.size() < 10) // 10 questions
    {
        int index = dis(g);
        if (selected_indices.count(index) == 0)
        {
            selected_indices.insert(index);
            selected_questions.push_back(questions[index]);
        }
    }

    return selected_questions;
}