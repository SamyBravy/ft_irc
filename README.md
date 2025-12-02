# ft_irc: Internet Relay Chat Server

[![Standard](https://img.shields.io/badge/Standard-C++98-blue.svg)](https://isocpp.org/)
[![Network](https://img.shields.io/badge/Protocol-TCP%2FIP-orange.svg)]()
[![Architecture](https://img.shields.io/badge/Architecture-Non--Blocking%20I%2FO-green.svg)]()

## Abstract

**ft_irc** is a complete implementation of an Internet Relay Chat (IRC) server, written in C++98. It is designed to demonstrate mastery of low-level networking, socket programming, and protocol parsing as defined in **RFC 1459**.

The system is built on a **single-threaded, non-blocking I/O architecture** using `poll()`. This design choice ensures the server handles multiple concurrent connections efficiently without the resource overhead of multi-threading, maintaining stability even under heavy load or partial data transmission.

## Installation

### Prerequisites
*   C++ Compiler (`g++` or `clang++`)
*   `make`
*   Any IRC Client (HexChat recommended)

### Compilation
The project utilizes a custom `Makefile` to manage builds for both the server and the automated service agent (bot).

| Command | Action |
|:---:|---|
| `make` | Compiles **both** the Server and the Bot. |
| `make ircserv` | Compiles only the **Server**. |
| `make ircbot` | Compiles only the **Bot**. |
| `make fclean` | Removes all object files and executables. |

## Usage

### 1. Start the Server
The server listens on a specified port and requires a password for incoming connections.

```bash
./ircserv <port> <password>
```
*Example:* `./ircserv 6667 securepass`

### 2. Connect a Client
While raw TCP connections (`nc`) work, a dedicated client provides the best testing environment.

#### Recommended: HexChat
**HexChat** is highly recommended for testing channel modes and operator privileges visually.

1.  Open HexChat and navigate to **Network List** (`Ctrl+S`).
2.  Add a new network named `ft_irc`.
3.  Edit the server details:
    *   **Server:** `localhost/6667` (or your port).
    *   **Password:** The password used to launch the server.
    *   **SSL:** Disabled.
4.  Connect.

### 3. Launch the Bot (Optional)
The **BravyBot** is an autonomous agent that connects to the server to provide utility services.

```bash
./ircbot <ip> <port> <password>
```
*Example:* `./ircbot 127.0.0.1 6667 securepass`

## Features & Implementation

### Technical Architecture
*   **I/O Multiplexing:** Uses `poll()` to monitor all file descriptors (sockets) within a single event loop.
*   **Non-Blocking:** All sockets are set to non-blocking mode via `fcntl()`.
*   **Memory Safety:** Strict RAII compliance (C++98) ensuring zero memory leaks.

### Server Commands
The server supports the standard IRC lifecycle including:
*   **Auth:** `CAP`, `PASS`, `NICK`, `USER`
*   **Operations:** `JOIN`, `PART`, `TOPIC`, `PRIVMSG`, `NOTICE`
*   **Management:** `KICK`, `INVITE`, `QUIT`, `PING/PONG`

### Channel Modes
Operators can control channel behavior using the `MODE` command:

| Flag | Description |
|:---:|---|
| **i** | **Invite-only:** Only invited users can join. |
| **t** | **Protected Topic:** Only operators can change the topic. |
| **k** | **Key:** Requires a password to join. |
| **o** | **Operator:** Grants/revokes operator status. |
| **l** | **Limit:** Sets a maximum number of users. |

### Automated Agent (Bot)
The `BravyBot` automatically joins `#BravyBotChannel` upon connection and interacts via private messages or channel chat.

**Available Commands:**
*   `!help` : Displays available commands.
*   `!time` / `!date` : Returns the current server time/date.
*   `!moment` : Returns full timestamp.
*   `!add <cmd> <msg>` : Dynamically teaches the bot a new custom command.
*   `!quit` : Disconnects the bot.

**Behaviors:**
*   **Auto-Join:** Automatically accepts invites to new channels.
*   **Greetings:** Welcomes users when they join a channel.
*   **Farewells:** Acknowledges users leaving a channel.