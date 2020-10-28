#include "./tracker.h"

using namespace std;

int main(int argc, char ** argv){
    
    if (argc < 2){
        cout << "Usage: ./tracker port_no" << endl;
        exit(1);
    }

    port = atoi(argv[1]);
    createServerSocket(&server_socket, port);

    cout << "Tracker online at port " << port << endl;

    pthread_t serverThread;

    pthread_create(&serverThread, NULL, listenFunc, NULL);

    pthread_join(serverThread, NULL);


    return 0;
}