#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
    using socklen_t = int;
    #define CLOSESOCK closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define INVALID_SOCKET (-1)
    #define SOCKET_ERROR   (-1)
    using SOCKET = int;
    #define CLOSESOCK close
#endif

constexpr uint16_t UDP_PORT = 8000;
constexpr uint16_t TCP_PORT = 9000;
using SockAddr = sockaddr_in;

std::unordered_map<std::string, std::string> load_db(const std::string& file) {
    std::ifstream fin(file);
    std::unordered_map<std::string, std::string> db;
    std::string domain, ip;
    while (fin >> domain >> ip) db[domain] = ip;
    return db;
}

int main() {
#ifdef _WIN32
    WSADATA wsa;  WSAStartup(MAKEWORD(2,2), &wsa);
#endif

    SOCKET udp = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp == INVALID_SOCKET) { return 1; }

#ifndef _WIN32
    int yes = 1;
    setsockopt(udp, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
#endif

    SockAddr udp_addr{};
    udp_addr.sin_family      = AF_INET;
    udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    udp_addr.sin_port        = htons(UDP_PORT);

    if (bind(udp, (sockaddr*)&udp_addr, sizeof(udp_addr)) == SOCKET_ERROR) {
        return 1;
    }

    auto local_db = load_db("local_dns.txt");

    char buf[512];
    for (;;) {
        SockAddr client {}; socklen_t clen = sizeof(client);
        int len = recvfrom(udp, buf, sizeof(buf)-1, 0,
                           (sockaddr*)&client, &clen);
        if (len <= 0) continue;
        buf[len] = '\0';
        std::string domain(buf);

        std::string answer;
        auto it = local_db.find(domain);
        if (it != local_db.end()) {
            answer = it->second;
        }
        /* — Case 2/3 : 글로벌 DNS 질의 — */
        else {
            SOCKET tcp = socket(AF_INET, SOCK_STREAM, 0);
            if (tcp == INVALID_SOCKET) { answer = "Not Found"; }
            else {
                SockAddr gaddr{};
                gaddr.sin_family = AF_INET;
                inet_pton(AF_INET, "127.0.0.1", &gaddr.sin_addr);
                gaddr.sin_port = htons(TCP_PORT);

                if (connect(tcp, (sockaddr*)&gaddr, sizeof(gaddr)) != 0) {
                    answer = "Not Found";
                } else {
                    send(tcp, domain.c_str(), (int)domain.size(), 0);
                    int n = recv(tcp, buf, sizeof(buf)-1, 0);
                    buf[n > 0 ? n : 0] = '\0';
                    answer = buf;
                }
                CLOSESOCK(tcp);
            }
        }
        sendto(udp, answer.c_str(), (int)answer.size(), 0,
               (sockaddr*)&client, clen);
    }

    CLOSESOCK(udp);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
