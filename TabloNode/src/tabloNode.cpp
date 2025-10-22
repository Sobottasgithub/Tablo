#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>

#include "utils/socket.h"

using namespace std;

int main() {
    cout << "Update" << endl;
    Socket socket;
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cout << "Still alive" << endl;
    }
    return 0;
}
