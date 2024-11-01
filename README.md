# Oware Multiplayer

This project is multiplayer Oware game implemented in C. It allows users to connect, challenge each other, and play games. The game state is saved and can be loaded when users reconnect.

## Project Structure

- `client/`
    - `client.c` client implementation.
    - `Makefile`
- `src/`
    - `game.c`: Contains the game logic and functions to save and load game states.
    - `commands.c`: Contains functions to handle user commands, game requests, and user interactions.
    - `server.c`: Contains functions to handle server connections and invoke commands.
- `saved_games.txt`: Stores the saved game states.

## Features

- User login and username management.
- Displaying a list of connected users.
- Sending and accepting game requests.
- Saving and loading game states.
- Handling game logic and user interactions.
- And finally, play games

## How to Build

To build the project, use the following command (from root):
```sh
make
```

To run the server:
```sh
./src/server
```

To run the client, go to /client:
```sh
make && ./client
```


When done, and connection is established, the following message will be displayed:

```
-------Connected to server--------
Available commands:
/login - Log in to the server with a specified username.
/see_users - Display a list of currently logged-in users.
/game - Request to start a game with another user.
/accept - Accept a game request from another user.
/join - Join an available game after a request is accepted.
/exit - Disconnect from the server.
/help - Show this list of commands.
```


## How to Play

1. Log in to the server using the `/login` command.
2. Display a list of connected users using the `/see_users` command.
3. Send a game request to another user 
using the `/game` command (you will be prompted to choose the user). output example:
```
/login
Please enter your username: 

p2
Welcome, p2!

/game
Choose a user to challenge:
1 - p1
Enter the number of the user you want to challenge:

```
4. The other user will receive 
a game request and can accept it using the `/accept` command
(you will be prompted to accept one of the game requests). output example:

```
p1
Welcome, p1!

You have received a game request.
/accept
Game Requests:
1 - Game with p2

Please enter the request number to accept:
```

5. Once the game request is accepted, you can join the game using the `/join` command.

When the game starts, the board will be displayed as follows:
```
-------Your turn--------


Player p1 pits:
  4   4   4   4   4   4 
-----------------------
  4   4   4   4   4   4 
Player p2 pits:
Player p2, choose a pit (1-6) or (7) to abandon and lose: 
```

you can then make moves and play.

and while you're playing, the game is being saved in the following 
format to the saved_games.txt file:

```
Player 1: p2
Player 2: p1
Scores: 0 0
0 5 5 5 5 4
4 0 5 5 5 5
Status: 0
```

if the server crashes and is restarted,
the game state can be loaded automatically. And after the login
of both players, the game will be resumed from the last state.


server:
```
➜  PR_TP_Oware git:(oware-lite) ✗ ./src/server
game loaded, p2 vs p1
Loaded 1 saved games.
Server is listening on port 8083...
```

client1:
```
/login
Please enter your username: 

p1
Welcome, p1!

an unfinished game was found against p2, joining
```

client2:
```
/login
Please enter your username: 

p2
Welcome, p2!
an unfinished game was found against p1, joining
```

and both players are then back in the game loop