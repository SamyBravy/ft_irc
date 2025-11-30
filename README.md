
# ft_irc

This project involves creating a functioning IRC (Internet Relay Chat) server in C++ 98. The server is capable of handling multiple client connections simultaneously using non-blocking I/O and a single poll loop. It communicates with standard IRC clients (such as HexChat or nc) following the Internet Relay Chat Protocol.

## Description

The goal of `ft_irc` is to understand the inner workings of network protocols, socket programming, and TCP/IP communication. The server manages users, channels, and message relaying while adhering to the C++98 standard. It ensures stability and prevents hanging even under load.

## Requirements

*   GCC or Clang compiler supporting C++98
*   Make
*   A Unix-based operating system (Linux/macOS)

## Compilation

The project includes a `Makefile` to compile both the server and the bonus bot.

*   To compile the server and the bot:
    ```bash
    make
    ```

*   To compile only the server:
    ```bash
    make ircserv
    ```

*   To compile only the bot:
    ```bash
    make ircbot
    ```

*   To clean object files:
    ```bash
    make clean
    ```

*   To clean object files and executables:
    ```bash
    make fclean
    ```

## Usage

### Running the Server

The server requires a listening port and a connection password.

```bash
./ircserv <port> <password>
```

*   `<port>`: The port number to listen on (must be between 1024 and 49151).
*   `<password>`: The password required for clients to connect.

**Example:**
```bash
./ircserv 6667 mysecretpassword
```

### Running the Bot (Bonus)

The project includes a custom bot named `CaccaBot`. It connects to the server and provides utility commands.

```bash
./ircbot <ip> <port> <password>
```

*   `<ip>`: The IP address of the IRC server (e.g., 127.0.0.1).
*   `<port>`: The port the server is listening on.
*   `<password>`: The server's password.

**Example:**
```bash
./ircbot 127.0.0.1 6667 mysecretpassword
```

## Features

### Technical Implementation
*   **Non-blocking I/O**: Uses `fcntl` to set file descriptors to non-blocking mode.
*   **Polling**: Uses `poll()` to handle multiple file descriptors (clients and server socket) within a single thread.
*   **C++98 Compliance**: Adheres strictly to the standard without using external libraries (like Boost).

### Supported Commands
The server implements the following IRC commands:

*   **Authentication**: `CAP`, `PASS`, `NICK`, `USER`
*   **Channel Management**: `JOIN`, `PART`, `TOPIC`, `INVITE`, `KICK`
*   **Communication**: `PRIVMSG`, `NOTICE`
*   **Utility**: `PING`, `WHO`, `QUIT`, `MODE`

### Channel Modes
The `MODE` command supports the following channel flags:

*   `i`: Set/remove Invite-only channel.
*   `t`: Set/remove the restrictions of the TOPIC command to channel operators.
*   `k`: Set/remove the channel key (password).
*   `o`: Give/take channel operator privilege.
*   `l`: Set/remove the user limit to channel.

### Bot Features
The `CaccaBot` connects to the server, joins a default channel (`#CaccaBotChannel`), and responds to specific commands via private message or channel chat:

*   `!help`: Displays the list of available commands.
*   `!time`: Displays the current time.
*   `!date`: Displays the current date.
*   `!day`: Displays the current day of the week.
*   `!moment`: Displays the full date and time.
*   `!add <command> <message>`: Adds a custom command dynamically.
*   `!quit`: Disconnects the bot (if issued by the bot itself) or makes it leave the channel.

The bot also interacts automatically when:
*   Invited to a channel (joins automatically).
*   A user joins a channel (greets them).
*   A user leaves a channel (sends a farewell message).
