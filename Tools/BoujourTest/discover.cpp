//
// Created by aojoie on 9/11/2023.
//

#include <winsock2.h>
#include <ws2tcpip.h>

#include <Core/MDNSService.h>
#include <iostream>
#include <string>

using namespace std;

void WaitInput() {
    char a;
    cin >> a;
}


std::string GetIPv4Address(const char* hostname) {
//    // Initialize Winsock
//    WSADATA wsaData;
//    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//        return "WSAStartup failed.";
//    }

    // Define hints for getaddrinfo
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4 only
    hints.ai_socktype = SOCK_STREAM; // TCP socket type

    // Get address info
    struct addrinfo* result = nullptr;
    int res = getaddrinfo(hostname, nullptr, &hints, &result);
    if (res != 0) {
        return {};
    }

    std::string ipv4Address;

    // Iterate through the list of addresses
    for (struct addrinfo* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        if (ptr->ai_family == AF_INET) {
            struct sockaddr_in* ipv4 = (struct sockaddr_in*)ptr->ai_addr;
            char ipstr[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr)) != nullptr) {
                ipv4Address = ipstr;
                break; // Get the first IPv4 address and exit the loop
            }
        }
    }

    // Clean up
    freeaddrinfo(result);
//    WSACleanup();

    return ipv4Address;
}

int main()
{

    AN::MDNSService service;

    service.SetDiscoverErrorCallback(
            [](const AN::MDNSError &error, void *userInfo)
            {
                cout << "MDNS Error " << error.message << endl;
            });

    service.StartDiscover("_anphonetool._tcp",
                          [](const char *name, AN::MDNSDiscoverState state, const char *hosttarget, uint16_t port, const AN::MDNSError &error, void *userInfo)
                          {
                               if (state == AN::kMDNSDiscoverAdd)
                               {
                                   cout << name << ' ' << hosttarget << ' ' << port << endl;
                                   cout << "ip address " << GetIPv4Address(hosttarget) << endl;
                               }
                               else
                               {
                                   cout << name << "removed" << endl;
                               }
                          }, nullptr);

    WaitInput();

    service.StopDiscover();

    return 0;
}