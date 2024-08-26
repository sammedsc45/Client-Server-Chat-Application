# NetChat: Multi-Client Chat Application

## 🌐 Overview

NetChat is a robust, multi-threaded client-server chat application written in C. It demonstrates advanced networking concepts, efficient socket programming, and concurrent client handling. This application allows multiple users to communicate in real-time over a network, supporting both broadcast and private messaging functionalities.

## ✨ Features

- 🔊 **Broadcast Messaging**: Send messages to all connected clients
- 📨 **Private Messaging**: Communicate directly with specific users
- 👥 **User Listing**: View all currently connected clients
- 🔒 **Unique Nicknames**: Ensure each user has a distinct identifier
- 🆘 **Help System**: Access available commands easily
- 🧵 **Multi-threaded**: Handle multiple clients concurrently
- 🔌 **Graceful Disconnection**: Proper handling of client disconnects

## 🛠 Technologies

- **Language**: C (C99 standard)
- **Networking**: TCP/IP sockets
- **Concurrency**: POSIX threads (pthreads)
- **Build System**: Make
- **Operating Systems**: Linux, macOS, and other UNIX-like systems

## 📋 Prerequisites

- GCC Compiler (version 7.0+)
- POSIX-compliant operating system
- Make build system
- Basic understanding of terminal/command-line interface

## 🚀 Getting Started

### Installation

1. Clone the repository:
2. Compile the project:
```bash
gcc server.c -o server -pthread
gcc client.c -o client
```
This will create two executables: `server` and `client`.

### Running the Server

1. Start the server:
```bash
./server
```
2. The server will display a message indicating it's running and listening for connections.

### Running the Client

1. Start a client instance:
```bash
./client
```

2. Enter a unique nickname when prompted.

## 🎮 Usage

Once connected, clients can use the following commands:

- `/broadcast <message>`: Send a message to all clients
- `/private <nickname> <message>`: Send a private message
- `/list`: Display all connected clients
- `/help`: Show available commands
- `/quit`: Disconnect from the server

## 🤝 Contributing

We welcome contributions! Please follow these steps:

1. Fork the repository
2. Create a new branch: `git checkout -b feature-branch-name`
3. Make your changes and commit them: `git commit -m 'Add some feature'`
4. Push to the branch: `git push origin feature-branch-name`
5. Submit a pull request

## 📜 License

This project is licensed under the MIT License - see the [MIT-Licence](LICENSE) file for details.


## 📞 Contact

For any queries or suggestions, please open an issue or contact the maintainer:

Sammed - [@SammedC94865641](https://x.com/SammedC94865641) - sammedchougule321@gmail.com

---

Happy chatting! 🎉

