#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <csignal>

using namespace std;

static mutex mtx;
static condition_variable cv;

template<typename T, size_t N>
class CircularBuffer {
private:
    T buf_[N];
    size_t head_{0}, tail_{0}, count_{0};

public:
    bool push(T value) {
        if ((tail_ + 1) % N == head_) return false;
        buf_[tail_++] = value;
        ++count_;
        tail_ %= N;
        return true;
    }

    bool pop(T &value) {
        if (empty()) return false;
        value = buf_[head_++];
        ++head_;
        head_ %= N;
        --count_;
        return true;
    }

    size_t length() const { return count_; }

    bool empty() const { return !length(); }
};

void signalHandler(int sig) {
    cout << "\nInterrupted! Shutting down..." << endl;
    lock_guard<mutex> lck(mtx);
    cv.notify_all();
    exit(sig);
}

void* handleClient(void *arg) {
    int clientSocket = *(int *) arg;
    free(arg);

    struct sockaddr_in clientAddress;
    socklen_t addrLen = sizeof(clientAddress);
    getsockname(clientSocket, (struct sockaddr *)&clientAddress, &addrLen);

    cout << "[Server]: New client connected [" << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << "]" << endl;

    CircularBuffer<char, 4096> receiveBuf_;

    while (true) {
        char recvBuff[4096];
        ssize_t bytesReceived = recv(clientSocket, recvBuff, sizeof(recvBuff), MSG_DONTWAIT);

        if (bytesReceived > 0) {
            for (ssize_t i = 0; i < bytesReceived; ++i) {
                if (!receiveBuf_.push(recvBuff[i])) {
                    perror("Error adding received data to buffer");
                    break;
                }
            }
            send(clientSocket, "ACK\r\n", strlen("ACK\r\n"), 0);
        }

        usleep(100000);
    }

    close(clientSocket);
    cout << "[Server]: Client disconnected [" << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << "]" << endl;
    return nullptr;
}

int main() {
    signal(SIGINT, signalHandler); // Set up signal handler for interrupt signals

    int listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(8080);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenSocket, (struct sockaddr *) &servAddr, sizeof(servAddr));
    listen(listenSocket, 3);

    cout << "[Server]: Listening on port 8080." << endl;

    while (true) {
        int clientSocket = accept(listenSocket, NULL, NULL);
        if (clientSocket >= 0) {
            thread clientThread(handleClient, (void *) &clientSocket);
            clientThread.detach();
        }
    }
}