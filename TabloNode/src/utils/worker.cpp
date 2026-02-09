
#include "worker.h"
#include <iostream>

#include <vector>
#include <mutex>

#include "tabnet.h"

void Worker::solveOrderCycle() {
    
}

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
