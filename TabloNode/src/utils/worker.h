#ifndef WORKER_H
#define WORKER_H

#include <server_session_controller.h>
#include "csv_manager.h"

#include <vector>
#include <mutex>

class Worker
{
    public:
        // Cycle
        void solveRequestCycle();

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

        CsvManager csvManager;

        // Logic functions
        ttp2::ServerSessionController::Packet test(ttp2::ServerSessionController::Packet packet);
        void setFile(ttp2::ServerSessionController::File newFile);
        ttp2::ServerSessionController::Packet getViewport(ttp2::ServerSessionController::Viewport viewportRequest);
};

#endif
