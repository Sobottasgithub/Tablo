#include <string>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <net/if.h>
#include "network_helpers.h"

using namespace std;

std::string NetworkHelpers::getLocalIpAddress() {
    struct ifaddrs *ifaddr = nullptr;

    // Get linked list of network interfaces
    if (getifaddrs(&ifaddr) == -1) {
        return "";
    }

    std::string result;

    // Iterate through interfaces
    for (auto *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            auto *addr = reinterpret_cast<struct sockaddr_in *>(ifa->ifa_addr);
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));

            // Docker containers typically use eth0
            if (std::string(ifa->ifa_name) == "eth0") {
                result = ip;
                break;
            }
        }
    }

    freeifaddrs(ifaddr);
    return result;
}

std::string NetworkHelpers::getBroadcastIpAddress() {
    struct ifaddrs* ifaddr = nullptr;
    std::string broadcastIP;

    // Get network interfaces
    if (getifaddrs(&ifaddr) == -1) {
        return "";
    }

    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        // Only consider IPv4 interfaces that are up and support broadcast
        if (ifa->ifa_addr->sa_family == AF_INET &&
            (ifa->ifa_flags & IFF_BROADCAST) &&
            (ifa->ifa_flags & IFF_UP) &&
            !(ifa->ifa_flags & IFF_LOOPBACK)) {

            // Ensure the broadcast address exists
            if (ifa->ifa_broadaddr) {
                struct sockaddr_in* bcast =
                    reinterpret_cast<struct sockaddr_in*>(ifa->ifa_broadaddr);
                char ip[INET_ADDRSTRLEN];
                if (inet_ntop(AF_INET, &(bcast->sin_addr), ip, INET_ADDRSTRLEN)) {
                    broadcastIP = ip;
                    break; // stop at the first valid one
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    return broadcastIP;
}
