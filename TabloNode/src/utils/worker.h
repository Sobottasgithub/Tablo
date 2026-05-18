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
        
        ServerSessionController::Packet getResponse();
        void pushResponse(ServerSessionController::Packet packet);

        int getResponseCollectionSize();
        int getRequestCollectionSize();
        
    private:
        std::mutex mtx;
        std::vector<ServerSessionController::Packet> responses;
        std::vector<ServerSessionController::Packet> requests;
};

#endif
