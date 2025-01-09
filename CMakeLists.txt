cmake_minimum_required(VERSION 3.10)

# Nazwa projektu
project(QuizServer)

# Ustawienia kompilatora
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Dodanie plików źródłowych
set(SOURCES
    serverFolder/main.cpp
    serverFolder/game.cpp
    serverFolder/server_utils.cpp
    serverFolder/nickname_handler.cpp
    serverFolder/questions.cpp
    serverFolder/player.cpp
)

# Dodanie plików nagłówkowych
set(HEADERS
    serverFolder/game.h
    serverFolder/server_utils.h
    serverFolder/nickname_handler.h
    serverFolder/questions.h
    serverFolder/player.h
)

# Tworzenie pliku wykonywalnego
add_executable(QuizServer ${SOURCES} ${HEADERS})

# Dodanie flag kompilatora
target_compile_options(QuizServer PRIVATE -Wall -Wextra -Wpedantic)

# Dodanie bibliotek systemowych
target_link_libraries(QuizServer PRIVATE pthread)