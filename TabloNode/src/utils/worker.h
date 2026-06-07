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
        ttp2::ServerSessionController::Packet test(ttp2::ServerSessionController::Packet packet);
        void setFile(ttp2::ServerSessionController::File newFile);

        // Service logic
        ttp2::ServerSessionController::Packet getRequest();
        void pushRequest(ttp2::ServerSessionController::Packet packet);
        
        ttp2::ServerSessionController::Packet getResponse();
        void pushResponse(ttp2::ServerSessionController::Packet packet);

        int getResponseCollectionSize();
        int getRequestCollectionSize();
        
    private:
        std::mutex mtx;
        std::vector<ttp2::ServerSessionController::Packet> responses;
        std::vector<ttp2::ServerSessionController::Packet> requests;

        ttp2::ServerSessionController::File file;
};

#endif
