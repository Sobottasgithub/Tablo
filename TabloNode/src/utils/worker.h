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
        
    private:
        vector<tabnet::Packet> solutions;
        vector<tabnet::Packet> orders;
};

#endif
