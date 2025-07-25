
#include "worker.h"
#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <bits/stdc++.h>
#include <vector>

using namespace std;

Worker::Worker() {
    cout << "";
}

void Worker::queTask(std::string message) {
    inQue.push_back(message);
}
std::string Worker::getOutput() {
    std::this_thread::sleep_for(90ms); // TODO: DELETE LATER
    std::string response = inQue[0];
    inQue.erase(inQue.begin());
    return response;
}