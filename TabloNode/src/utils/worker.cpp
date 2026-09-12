#include "worker.h"

#include <server_session_controller.h>
#include <packet_types.h>

#include "csv_manager.h"

#include <tablog.h>

#include <vector>
#include <mutex>
#include <variant>
#include <thread>
#include <optional>

// Cycle
void Worker::solveRequestCycle() {
    connected = true;
    if (this->isCalled == true) {
        logger->log(tablog::ERROR, "SolveRequestCycle is already called!");
        return;
    }
    this->isCalled = true;
    
    while (isConnected()) {
        int requestSize = getRequestCollectionSize();
        for (int count = 0; count < requestSize; count++) {
            ttp2::Packet::Packet request = getRequest();

            if (std::holds_alternative<ttp2::Packet::Standard>(request.payload)) {
                pushResponse(Worker::test(request));
            } else if (std::holds_alternative<ttp2::Packet::File>(request.payload)) {
                logger->log(tablog::DEBUG, "id: " + std::to_string(request.id));
                ttp2::Packet::File file = std::get<ttp2::Packet::File>(request.payload);
                Worker::setFile(file);
            } else if (std::holds_alternative<ttp2::Packet::Viewport>(request.payload)) {
                logger->log(tablog::CRITICAL, "Undefined behavior for the viewport");
            } else if (std::holds_alternative<ttp2::Packet::ViewportRequest>(request.payload)) {
                ttp2::Packet::ViewportRequest viewportRequest = std::get<ttp2::Packet::ViewportRequest>(request.payload);
                ttp2::Packet::Packet responsePacket = Worker::getViewport(viewportRequest);
                responsePacket.id = request.id;
                pushResponse(responsePacket);
            } else if (std::holds_alternative<ttp2::Packet::TqlQuery>(request.payload)) {
                ttp2::Packet::TqlQuery queryRequest = std::get<ttp2::Packet::TqlQuery>(request.payload);
                executeQuery(queryRequest);
            } else {
                logger->log(tablog::CRITICAL, "Unknown payload type!");
            }
        }
        std::this_thread::yield();
    }
}

// Logic functions
ttp2::Packet::Packet Worker::test(ttp2::Packet::Packet packet) {
    return packet;
}

void Worker::setFile(ttp2::Packet::File newFile) {
    this->csvManager.setFile(newFile);
}

ttp2::Packet::Packet Worker::getViewport(ttp2::Packet::ViewportRequest viewportRequest) {
    ttp2::Packet::Packet packet;
    ttp2::Packet::Viewport viewport;

    if (viewportRequest.xEnd < viewportRequest.xStart || viewportRequest.yEnd < viewportRequest.yStart) {
        ttp2::Packet::Viewport emptyViewport;
        packet.payload = emptyViewport;
        return packet;
    }

    viewport.xStart = viewportRequest.xStart;
    viewport.xEnd = viewportRequest.xEnd;
    viewport.yStart = viewportRequest.yStart;
    viewport.yEnd = viewportRequest.yEnd;
    viewport.payload = this->csvManager.getViewport(viewportRequest.xStart, viewportRequest.xEnd,
                                                           viewportRequest.yStart, viewportRequest.yEnd);
    packet.payload = viewport;
    return packet;
}

void Worker::executeQuery(ttp2::Packet::TqlQuery queryRequest) {
    std::optional<ttp2::Packet::Error> optionalError = this->csvManager.executeQuery(queryRequest.query);

    if (optionalError.has_value()) {
        ttp2::Packet::Packet packet;
        packet.payload = optionalError.value();
        pushResponse(packet);
    }
}

// Service logic
ttp2::Packet::Packet Worker::getRequest() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!requests.empty()) {
        ttp2::Packet::Packet firstRequest = requests[0];
        requests.erase(requests.begin());
        return firstRequest;
    }
    ttp2::Packet::Packet emptyPacket;
    return emptyPacket;
}

void Worker::pushRequest(ttp2::Packet::Packet packet) {
    std::lock_guard<std::mutex> lock(mtx);
    requests.push_back(packet);
}

ttp2::Packet::Packet Worker::getResponse() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!responses.empty()) {
        ttp2::Packet::Packet firstResponse = responses[0];
        responses.erase(responses.begin());
        return firstResponse;
    }
    ttp2::Packet::Packet emptyPacket;
    return emptyPacket;
}

void Worker::pushResponse(ttp2::Packet::Packet packet) {
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

bool Worker::isConnected() {
    std::lock_guard<std::mutex> lock(mtx);
    return connected;
}

void Worker::disconnect() {
    std::lock_guard<std::mutex> lock(mtx);
    connected = false;
    isCalled = false;
}
