#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>

#pragma comment(lib, "ws2_32.lib")
#define LOCAL_PORT 8000
#define GLOBAL_PORT 9000
#define BUFFER_SIZE 512

std::unordered_map<std::string, std::string> loadLocalDB(const std::string& filename) {
    std::unordered_map<std::string, std::string> db;
    std::ifstream file(filename);
    std::string domain, ip;
    while (file >> domain >> ip) {
        db[domain] = ip;
    }
    return db;
}

int main() {
    WSADATA wsa;
    SOCKET udpSock, tcpSock;
    sockaddr_in localAddr, clientAddr, globalAddr;
    char buffer[BUFFER_SIZE];
    int clientAddrLen = sizeof(clientAddr);

    // Init Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // Load local DNS file
    auto localDB = loadLocalDB("local_dns.txt");

    // Setup UDP socket
    udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSock == INVALID_SOCKET) {
        std::cerr << "UDP socket creation failed\n";
        WSACleanup();
        return 1;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    localAddr.sin_port = htons(LOCAL_PORT);

    if (bind(udpSock, (sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) {
        std::cerr << "UDP bind failed\n";
        closesocket(udpSock);
        WSACleanup();
        return 1;
    }

    std::cout << "Local DNS server listening on UDP port " << LOCAL_PORT << "...\n";

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int len = recvfrom(udpSock, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrLen);
        if (len == SOCKET_ERROR) continue;

        std::string domain(buffer);
        std::string reply;

        // Case 1: Found in local
        if (localDB.find(domain) != localDB.end()) {
            reply = localDB[domain];
        } else {
            // Case 2: Not found → forward to global via TCP
            tcpSock = socket(AF_INET, SOCK_STREAM, 0);
            if (tcpSock == INVALID_SOCKET) continue;

            globalAddr.sin_family = AF_INET;
            globalAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
            globalAddr.sin_port = htons(GLOBAL_PORT);

            if (connect(tcpSock, (sockaddr*)&globalAddr, sizeof(globalAddr)) == SOCKET_ERROR) {
                closesocket(tcpSock);
                continue;
            }

            send(tcpSock, domain.c_str(), domain.length(), 0);
            memset(buffer, 0, BUFFER_SIZE);
            recv(tcpSock, buffer, BUFFER_SIZE, 0);
            reply = buffer;

            closesocket(tcpSock);
        }

        // Send response back to client
        sendto(udpSock, reply.c_str(), reply.length(), 0, (sockaddr*)&clientAddr, clientAddrLen);
    }

    closesocket(udpSock);
    WSACleanup();
    return 0;
}
