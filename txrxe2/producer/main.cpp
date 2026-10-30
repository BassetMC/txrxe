#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <vector>
#include <atomic>
#include <thread>
#include "core/txrx.h"


std::atomic<bool> flag{true};

int PACKET_SIZE = 4096;
const char* fifoPath = "/dev/shm/txrxe1";

void task() {
    for (;;) {
        if (!flag.load(std::memory_order_relaxed)) {
            continue;
        }

        int fd = open(fifoPath, O_WRONLY);
        if (fd == -1) {
            perror("open");
            _exit(1);
        }

        std::vector<char> msg(PACKET_SIZE, 'A');

        ssize_t n = write(fd, msg.data(), msg.size());
        if (n == -1) {
            perror("write");
        }

        close(fd);
    }

}


int main() {

    std::thread t(task);

    for (;;) {
        int key = getch_unix();

        if (key == 27) { // 27 — ASCII-код клавіші ESC
            std::cout << "Натиснуто ESC. Вихід з програми.\n";
            break;
        }
        else {
            flag = !flag;
            std::cout << "Натиснуто не ESC. flag зараз: " << flag <<"\n";
        }

    }

    return 0;
}
