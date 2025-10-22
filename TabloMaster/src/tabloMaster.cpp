#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <ctime>

#include "utils/socket.h"

using namespace std;

int main()
{
    cout << "Update" << endl;
    Socket socket;
    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cout << "Still alive" << endl;
    }
    return 0;
}

