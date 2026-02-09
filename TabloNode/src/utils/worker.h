#ifndef WORKER_H
#define WORKER_H

#include <vector>

#include "tabnet.h"

class Worker
{
    public:
        void solveOrderCycle();
        
        tabnet::Packet getOrder();
        void pushOrder(tabnet::Packet packet);
        
        tabnet::Packet getSolution();
        void pushSolution(tabnet::Packet packet);

        int getSolutionCollectionSize();
        int getOrderCollectionSize();
        
    private:
        std::mutex mtx;
        vector<tabnet::Packet> solutions;
        vector<tabnet::Packet> orders;
};

#endif
