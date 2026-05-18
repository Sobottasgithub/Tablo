#include "worker.h"

#include <server_session_controller.h>

#include <iostream>
#include <vector>
#include <mutex>

// Cycle
void Worker::solveRequestCycle() {
    while (true) {
        int requestSize = getRequestCollectionSize();
        if (requestSize > 0) {
            for (int count = 0; count < requestSize; count++) {
                ServerSessionController::Packet request = getRequest();

                if (std::holds_alternative<ServerSessionController::Standard>(request.payload)) {
                        pushResponse(Worker::test(request));
                } else {
                    std::wcout << "Unknown payload type!" << std::endl;                    
                }
            }
        }
    }
}

// Logic functions
ServerSessionController::Packet Worker::test(ServerSessionController::Packet packet) {
    return packet;
}

ServerSessionController::Packet Worker::setFile(ServerSessionController::Packet packet) {
    return packet;
}

// Service logic
ServerSessionController::Packet Worker::getRequest() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!requests.empty()) {
        ServerSessionController::Packet firstRequest = requests[0];
        requests.erase(requests.begin());
        return firstRequest;
    }
    ServerSessionController::Packet emptyPacket;
    return emptyPacket;
}

void Worker::pushRequest(ServerSessionController::Packet packet) {
    std::lock_guard<std::mutex> lock(mtx);
    requests.push_back(packet);
}

ServerSessionController::Packet Worker::getResponse() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!responses.empty()) {
        ServerSessionController::Packet firstResponse = responses[0];
        responses.erase(responses.begin());
        return firstResponse;
    }
    ServerSessionController::Packet emptyPacket;
    return emptyPacket;
}

void Worker::pushResponse(ServerSessionController::Packet packet) {
    std::lock_guard<std::mutex> lock(mtx);
    responses.push_back(packet);
}

int Worker::getResponseCollectionSize() {
    std::lock_guard<std::mutex> lock(mtx);
    return responses.size();
}

int Worker::getRequestCollectionSize() {
    std::lock_guard<std::mutex> lock(mtx);
    return requests.size();
}
