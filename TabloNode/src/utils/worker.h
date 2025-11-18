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
        std::string testCycle(std::string data);
};

#endif
