#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define PORT 8080

void save_to_history(const std::string& msg) {
    std::ofstream file("chat_log.txt", std::ios::app);
    if (file.is_open()) {
        file << msg << std::endl;
    }
}

int main() {
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) {
        perror("socket");
        return 1;
    }
    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listener, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind");
        close(listener);
        return 1;
    }
    
    if (listen(listener, 1) < 0) {
        perror("listen");
        close(listener);
        return 1;
    }
    std::cout << "Сервер запущен (порт 8080). Ожидание подключений...\n";
    while (true) {
        int client_sock = accept(listener, nullptr, nullptr);
        if (client_sock < 0) {
            perror("accept");
            continue;
        }
        std::cout << "[+] Клиент подключился\n";
		bool flag = true;
        char buffer[512];
        while (true) {
            int bytes = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes <= 0) {
                std::cout << "[-] Клиент отключился\n";
                close(client_sock);
                flag = false;
                break;
            }
			buffer[bytes] = '\0';
            std::string msg(buffer);
            if (!msg.empty() && msg.back() == '\n') {
                msg.pop_back();
            }
            std::cout << "Клиент: " << msg << "\n";
            save_to_history("Клиент: " + msg);
            std::string reply;
            if (msg == "ping") {
                reply = "pong";
            } else {
                reply = msg;
            }
            save_to_history("Сервер: " + reply);
            std::string wire = reply + "\n";
            if (send(client_sock, wire.c_str(), wire.size(), 0) <= 0) {
                perror("send");
                close(client_sock);
                flag = false;
                break;
            }
        }
        if (!flag) {
        	break;
        }
    }
    close(listener);
    return 0;
}
