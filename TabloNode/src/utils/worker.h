#ifndef WORKER_H
#define WORKER_H

#include <vector>

#include "tabnet.h"

class Worker
{
    public:
        // Cycle
        void solveOrderCycle();

        // Logic functions
        tabnet::Packet test(tabnet::Packet packet);
        tabnet::Packet setFile(tabnet::Packet packet);

        // Service logic
        tabnet::Packet getOrder();
        void pushOrder(tabnet::Packet packet);
        
        tabnet::Packet getSolution();
        void pushSolution(tabnet::Packet packet);

        int getSolutionCollectionSize();
        int getOrderCollectionSize();
        
    private:
        std::mutex mtx;
        std::vector<tabnet::Packet> solutions;
        std::vector<tabnet::Packet> orders;
};

#endif
