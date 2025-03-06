# Quiz Game Project

## Description

This project is a multiplayer quiz game where players can join a server, answer questions, and compete for the highest score. The server handles multiple clients, manages the game state, and ensures fair play. The client provides a user-friendly interface for players to connect to the server, answer questions, and view the leaderboard.

## Images

### Joining to the game (two players are waiting for the third one to start the game).
![Joining](https://www.janwysmolinski.pl/projects/gqp/1.png)

### Game is in progress.
![Game in progress](https://www.janwysmolinski.pl/projects/gqp/2.png)

### One player left, but the other two players still compete.
![One player left](https://www.janwysmolinski.pl/projects/gqp/3.png)

## Compilation

To compile the server, use the following commands:

```bash
mkdir build
cd build
cmake ..
make
```

## Usage

### Running the Server

To run the server, use:

```bash
./build/QuizServer
```

### Running the Client

To run the client, use:

```bash
python clientFolder/client.py
```

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
