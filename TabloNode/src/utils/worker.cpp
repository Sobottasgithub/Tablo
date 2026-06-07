#include "worker.h"

#include <server_session_controller.h>

#include <iostream>
#include <vector>
#include <mutex>
#include <variant>

// Cycle
void Worker::solveRequestCycle() {
    while (true) {
        int requestSize = getRequestCollectionSize();
        if (requestSize > 0) {
            for (int count = 0; count < requestSize; count++) {
                ttp2::ServerSessionController::Packet request = getRequest();

                if (std::holds_alternative<ttp2::ServerSessionController::Standard>(request.payload)) {
                    pushResponse(Worker::test(request));
                } else if (std::holds_alternative<ttp2::ServerSessionController::File>(request.payload)) {
                    ttp2::ServerSessionController::File file = std::get<ttp2::ServerSessionController::File>(request.payload);
                    Worker::setFile(file);
                } else {
                    std::wcout << "Unknown payload type!" << std::endl;                    
                }
            }
        }
    }
}

// Logic functions
ttp2::ServerSessionController::Packet Worker::test(ttp2::ServerSessionController::Packet packet) {
    return packet;
}

void Worker::setFile(ttp2::ServerSessionController::File newFile) {
    file = newFile;
    std::wcout << file.payload.c_str() << std::endl;
}

// Service logic
ttp2::ServerSessionController::Packet Worker::getRequest() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!requests.empty()) {
        ttp2::ServerSessionController::Packet firstRequest = requests[0];
        requests.erase(requests.begin());
        return firstRequest;
    }
    ttp2::ServerSessionController::Packet emptyPacket;
    return emptyPacket;
}

void Worker::pushRequest(ttp2::ServerSessionController::Packet packet) {
    std::lock_guard<std::mutex> lock(mtx);
    requests.push_back(packet);
}

ttp2::ServerSessionController::Packet Worker::getResponse() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!responses.empty()) {
        ttp2::ServerSessionController::Packet firstResponse = responses[0];
        responses.erase(responses.begin());
        return firstResponse;
    }
    ttp2::ServerSessionController::Packet emptyPacket;
    return emptyPacket;
}

void Worker::pushResponse(ttp2::ServerSessionController::Packet packet) {
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
