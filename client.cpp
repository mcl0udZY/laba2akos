#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 8080

void show_history() {
    std::ifstream file("chat_log.txt");
    if (!file) {
        std::cout << "\nИстория пока пуста.\n";
        return;
    }
    std::cout << "\n=== История переписки ===\n";
    std::string line;
    while (std::getline(file, line)) {
        std::cout << line << '\n';
    }
    std::cout << "=== Конец истории ===\n";
}

int main() {
    int client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0) {
        std::cerr << "Не удалось создать сокет\n";
        return 1;
    }
    sockaddr_in server{};
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) <= 0) {
        std::cerr << "inet_pton() вернул ошибку\n";
        close(client_sock);
        return 1;
    }
    if (connect(client_sock, reinterpret_cast<sockaddr*>(&server), sizeof(server)) < 0) {
        std::cerr << "Ошибка подключения, сервер не доступен\n";
        close(client_sock);
        return 1;
    }
    std::cout << "Подключились к серверу 127.0.0.1:" << PORT << "\n";
    std::string ping = "ping\n";
	write(client_sock, ping.c_str(), ping.size());
	char buffer[512];
	ssize_t r = read(client_sock, buffer, sizeof(buffer) - 1);
	if (r > 0) {
		buffer[r] = '\0';
		std::cout << "Handshake: " << buffer;
	}
    std::cout << "Введите сообщение (\"/exit\" для выхода):\n";
    std::string msg;
    std::string response;
    while (std::getline(std::cin, msg)) {
        if (msg == "/exit") {
        	break;
        } 
        msg.push_back('\n');
        if (write(client_sock, msg.c_str(), msg.size()) <= 0) {
            std::cerr << "Ошибка отправки сообщения\n";
            close(client_sock);
            return 1;
        }
        r = read(client_sock, buffer, sizeof(buffer) - 1);
        if (r <= 0) {
            std::cerr << "Сервер закрыл соединение\n";
            break;
        }
        buffer[r] = '\0';
        std::cout << "Ответ сервера: " << buffer;
    }
    std::cout << "Отключаемся...\n";
    close(client_sock);
    show_history();
    return 0;
}