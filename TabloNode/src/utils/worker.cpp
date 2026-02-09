
#include "worker.h"
#include <iostream>

#include <vector>
#include <mutex>

#include "tabnet.h"
#include "worker.h"

// Cycle
void Worker::solveOrderCycle() {
    while (true) {
        orderSize = getOrderCollectionSize();
        if (orderSize > 0) {
            for (int count; count < orderSize; count++) {
                tabnet::Packet order = getOrder();

                switch (order.method) {
                    case Methods::test:
                        pushSolution(Worker::test(order));
                        break

                    case Methods::setFile:
                        pushSolution(Worker::setFile(order));
                        break;
                        
                    default:
                        std::wcout << "Unknown action: " << order.method << " -> Expected a method between " << Methods::START << " and " << Methods::END << std::endl;
                        break;
                }
            }
        }
    }
}

// Logic functions
tabnet::Packet Worker::test(tabnet::Packet packet) {
    tabnet::Packet solution;
    solution.method = Methods::response;
    solution.payload = packet.payload;
}

tabnet::Packet Worker::setFile(tabnet::Packet packet) {
    return packet;
}

// Service logic
tabnet::Packet Worker::getOrder() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!orders.empty()) {
        tabnet::Packet firstOrder = orders[0];
        orders.erase(orders.begin());
        return firstOrder;
    }
    tabnet::Packet emptyPacket;
    return emptyPacket;
}

void Worker::pushOrder(tabnet::Packet packet) {
    std::lock_guard<std::mutex> lock(mtx);
    orders.push_back(packet);
}

tabnet::Packet Worker::getSolution() {
    std::lock_guard<std::mutex> lock(mtx);
    if (!solutions.empty()) {
        tabnet::Packet firstSolution = solutions[0];
        solutions.erase(solutions.begin());
        return firstSolution;
    }
    tabnet::Packet emptyPacket;
    return emptyPacket;
}

void Worker::pushSolution(tabnet::Packet packet) {
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
