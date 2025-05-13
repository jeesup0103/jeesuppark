#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

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
    int udpSock, tcpSock;
    sockaddr_in localAddr{}, clientAddr{}, globalAddr{};
    char buffer[BUFFER_SIZE];
    socklen_t clientAddrLen = sizeof(clientAddr);

    // Load local DNS data
    auto localDB = loadLocalDB("local_dns.txt");

    // Create UDP socket
    udpSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSock < 0) {
        perror("UDP socket creation failed");
        return 1;
    }

    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    localAddr.sin_port = htons(LOCAL_PORT);

    if (bind(udpSock, (sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
        perror("UDP bind failed");
        close(udpSock);
        return 1;
    }

    std::cout << "Local DNS server listening on UDP port " << LOCAL_PORT << "...\n";

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t len = recvfrom(udpSock, buffer, BUFFER_SIZE, 0, (sockaddr*)&clientAddr, &clientAddrLen);
        if (len < 0) {
            perror("recvfrom failed");
            continue;
        }

        std::string domain(buffer);
        std::string reply;

        if (localDB.find(domain) != localDB.end()) {
            reply = localDB[domain];
        } else {
            // TCP to Global DNS
            tcpSock = socket(AF_INET, SOCK_STREAM, 0);
            if (tcpSock < 0) {
                perror("TCP socket creation failed");
                continue;
            }

            globalAddr.sin_family = AF_INET;
            globalAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
            globalAddr.sin_port = htons(GLOBAL_PORT);

            if (connect(tcpSock, (sockaddr*)&globalAddr, sizeof(globalAddr)) < 0) {
                perror("TCP connect failed");
                close(tcpSock);
                continue;
            }

            send(tcpSock, domain.c_str(), domain.length(), 0);
            memset(buffer, 0, BUFFER_SIZE);
            recv(tcpSock, buffer, BUFFER_SIZE, 0);
            reply = buffer;
            close(tcpSock);
        }

        // Respond via UDP
        sendto(udpSock, reply.c_str(), reply.length(), 0, (sockaddr*)&clientAddr, clientAddrLen);
    }

    close(udpSock);
    return 0;
}
