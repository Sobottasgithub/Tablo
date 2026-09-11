#ifndef WORKER_H
#define WORKER_H

#include <server_session_controller.h>
#include <packet_types.h>

#include "csv_manager.h"

#include <tablog_registry.h>
#include <tablog.h>

#include <mutex>
#include <vector>

class Worker
{
    public:
        // Cycle
        void solveRequestCycle();

        // Service logic
        ttp2::Packet::Packet getRequest();
        void pushRequest(ttp2::Packet::Packet packet);
        
        ttp2::Packet::Packet getResponse();
        void pushResponse(ttp2::Packet::Packet packet);

        int getResponseCollectionSize();
        int getRequestCollectionSize();

        bool isConnected();
        void disconnect();
        
    private:
        std::shared_ptr<tablog::Tablog> logger = tablog::TablogRegistry::getInstance().get("Tablo-Node");

        bool connected = false;
        bool isCalled = false;
        
        std::mutex mtx;
        std::vector<ttp2::Packet::Packet> responses;
        std::vector<ttp2::Packet::Packet> requests;

        CsvManager csvManager;

        // Logic functions
        ttp2::Packet::Packet test(ttp2::Packet::Packet packet);
        void setFile(ttp2::Packet::File newFile);
        ttp2::Packet::Packet getViewport(ttp2::Packet::ViewportRequest viewportRequest);
        void executeQuery(ttp2::Packet::TqlQuery queryRequest);
};

#endif
