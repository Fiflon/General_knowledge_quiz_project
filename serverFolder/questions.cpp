#include "questions.h"
#include <vector>
#include <algorithm>
#include <random>
#include <unordered_set>

Question questions[] = {
    {1,
     "What is the capital of Poland?",
     "Warsaw",
     "Krakow",
     "Gdansk",
     "Wroclaw",
     'a',
     1},
    {2,
     "What is the capital of France?",
     "Paris",
     "Lyon",
     "Marseille",
     "Nice",
     'a',
     1},
    {3,
     "What is the capital of Germany?",
     "Berlin",
     "Munich",
     "Hamburg",
     "Frankfurt",
     'a',
     1},
    {4,
     "What is the capital of Italy?",
     "Rome",
     "Milan",
     "Naples",
     "Turin",
     'a',
     1},
    {5,
     "What is the capital of Spain?",
     "Madrid",
     "Barcelona",
     "Valencia",
     "Seville",
     'a',
     1},
    {6,
     "What is the capital of Portugal?",
     "Lisbon",
     "Porto",
     "Vila Nova de Gaia",
     "Amadora",
     'a',
     1},
    {7,
     "What is the capital of the United Kingdom?",
     "London",
     "Birmingham",
     "Glasgow",
     "Liverpool",
     'a',
     1},
    {8,
     "What is the capital of the United States?",
     "Washington",
     "New York",
     "Los Angeles",
     "Chicago",
     'a',
     1},
    {9,
     "What is the capital of Canada?",
     "Ottawa",
     "Toronto",
     "Montreal",
     "Vancouver",
     'a',
     1},
    {10,
     "What is the capital of Brazil?",
     "Brasilia",
     "Sao Paulo",
     "Rio de Janeiro",
     "Salvador",
     'a',
     1},
    {11,
     "What is the capital of Australia?",
     "Canberra",
     "Sydney",
     "Melbourne",
     "Brisbane",
     'a',
     2},
    {12,
     "What is the capital of Japan?",
     "Tokyo",
     "Osaka",
     "Kyoto",
     "Nagoya",
     'a',
     2},
    {13,
     "What is the capital of China?",
     "Beijing",
     "Shanghai",
     "Guangzhou",
     "Shenzhen",
     'a',
     2},
    {14,
     "What is the capital of Russia?",
     "Moscow",
     "Saint Petersburg",
     "Novosibirsk",
     "Yekaterinburg",
     'a',
     2},
    {15,
     "What is the capital of India?",
     "New Delhi",
     "Mumbai",
     "Bangalore",
     "Chennai",
     'a',
     2},
    {16,
     "What is the capital of South Africa?",
     "Pretoria",
     "Cape Town",
     "Johannesburg",
     "Durban",
     'a',
     3},
    {17,
     "What is the capital of Argentina?",
     "Buenos Aires",
     "Cordoba",
     "Rosario",
     "Mendoza",
     'a',
     3},
    {18,
     "What is the capital of Egypt?",
     "Cairo",
     "Alexandria",
     "Giza",
     "Luxor",
     'a',
     3},
    {19,
     "What is the capital of Turkey?",
     "Ankara",
     "Istanbul",
     "Izmir",
     "Bursa",
     'a',
     3},
    {20,
     "What is the capital of Mexico?",
     "Mexico City",
     "Guadalajara",
     "Monterrey",
     "Puebla",
     'a',
     3}};

std::vector<Question> pick_questions()
{
    std::vector<Question> selected_questions;
    std::unordered_set<int> selected_indices;
    std::random_device rd;
    std::mt19937 g(rd());
    std::uniform_int_distribution<> dis(0, 19);

    while (selected_indices.size() < 10)
    {
        int index = dis(g);
        if (selected_indices.find(index) == selected_indices.end())
        {
            selected_indices.insert(index);
            selected_questions.push_back(questions[index]);
        }
    }

    return selected_questions;
}