#include "worker.h"

#include <server_session_controller.h>

#include <iostream>
#include <vector>
#include <mutex>

// Cycle
void Worker::solveOrderCycle() {
    while (true) {
        int orderSize = getOrderCollectionSize();
        if (orderSize > 0) {
            for (int count; count < orderSize; count++) {
                ServerSessionController::Packet order = getOrder();

                if (std::holds_alternative<ServerSessionController::Standard>(order.payload)) {
                        pushSolution(Worker::test(order));
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
ServerSessionController::Packet Worker::getOrder() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!orders.empty()) {
        ServerSessionController::Packet firstOrder = orders[0];
        orders.erase(orders.begin());
        return firstOrder;
    }
    ServerSessionController::Packet emptyPacket;
    return emptyPacket;
}

void Worker::pushOrder(ServerSessionController::Packet packet) {
    std::lock_guard<std::mutex> lock(mtx);
    orders.push_back(packet);
}

ServerSessionController::Packet Worker::getSolution() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!solutions.empty()) {
        ServerSessionController::Packet firstSolution = solutions[0];
        solutions.erase(solutions.begin());
        return firstSolution;
    }
    ServerSessionController::Packet emptyPacket;
    return emptyPacket;
}

void Worker::pushSolution(ServerSessionController::Packet packet) {
    std::lock_guard<std::mutex> lock(mtx);
    solutions.push_back(packet);
}

int Worker::getSolutionCollectionSize() {
    std::lock_guard<std::mutex> lock(mtx);
    return solutions.size();
}

int Worker::getOrderCollectionSize() {
    std::lock_guard<std::mutex> lock(mtx);
    return orders.size();
}
