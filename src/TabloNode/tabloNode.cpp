#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;
using namespace std;


#include "utils/socket.h"

int main(int argc, char * argv[]) {
    Socket socket(8080);
    socket.detatch();

    while(true) {
        string s = socket.getInput();
        for(int i = 0; i < 10000000; i++) {
            cout << "";
        }
        socket.setOutput(s);
    }

    cout << "TERMINATED";

    socket.join();
    
    return 0;

    //TODO: get not empty message
}