#ifndef WORKER_H
#define WORKER_H

#include <server_session_controller.h>

#include <vector>
#include <mutex>

class Worker
{
    public:
        // Cycle
        void solveRequestCycle();

        // Logic functions
        ServerSessionController::Packet test(ServerSessionController::Packet packet);
        ServerSessionController::Packet setFile(ServerSessionController::Packet packet);

        // Service logic
        ServerSessionController::Packet getRequest();
        void pushRequest(ServerSessionController::Packet packet);
        
        ServerSessionController::Packet getSolution();
        void pushSolution(ServerSessionController::Packet packet);

        int getSolutionCollectionSize();
        int getRequestCollectionSize();
        
    private:
        std::mutex mtx;
        std::vector<ServerSessionController::Packet> solutions;
        std::vector<ServerSessionController::Packet> requests;
};

#endif
