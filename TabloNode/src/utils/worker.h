#ifndef WORKER_H
#define WORKER_H

#include <server_session_controller.h>

#include <vector>
#include <mutex>

class Worker
{
    public:
        // Cycle
        void solveOrderCycle();

        // Logic functions
        ServerSessionController::Packet test(ServerSessionController::Packet packet);
        ServerSessionController::Packet setFile(ServerSessionController::Packet packet);

        // Service logic
        ServerSessionController::Packet getOrder();
        void pushOrder(ServerSessionController::Packet packet);
        
        ServerSessionController::Packet getSolution();
        void pushSolution(ServerSessionController::Packet packet);

        int getSolutionCollectionSize();
        int getOrderCollectionSize();
        
    private:
        std::mutex mtx;
        std::vector<ServerSessionController::Packet> solutions;
        std::vector<ServerSessionController::Packet> orders;
};

#endif
