#ifndef WORKER_H
#define WORKER_H

#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <bits/stdc++.h>
#include <vector>

class Worker
{
    public:
        Worker();
        void queTask(std::string);
        std::string getOutput();

    private:
        std::vector<std::string> inQue;
        std::vector<std::string> outQue;
};

#endif