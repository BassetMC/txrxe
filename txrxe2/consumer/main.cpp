#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#include <atomic>
#include <vector>
#include "core/txrx.h"

std::atomic<bool> flag{true};
std::atomic<uint64_t> total{0};
std::atomic<int> pps{0};
std::atomic<int> bps{0};
int PACKET_SIZE = 4096;
const char* fifoPath = "/dev/shm/txrxe1";

void task() {


    unlink(fifoPath);
    mkfifo(fifoPath, 0600);
    int fd = open(fifoPath, O_RDONLY);
    if (fd == -1) {
        perror("open");
        exit(1);
    }

    for (;;) {
        if (!flag.load(std::memory_order_relaxed)) {
            continue;
        }
        std::vector<char> buf(PACKET_SIZE);

        int res = read(fd, buf.data(), buf.size());
        if (res == 0) total++;

    }

}

void stats() {
    uint64_t prev_total = 0;

    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        uint64_t curr_total = total.load(std::memory_order_relaxed);
        uint64_t pps = curr_total - prev_total;
        uint64_t bps = pps * PACKET_SIZE;
        prev_total = curr_total;

        std::cout << "total: " << curr_total << '\n'
                  << "pps:   " << pps << " pkt/s\n"
                  << "bps:   " << bps << " B/s\n\n"
                  << std::flush;
    }
}


int main() {


    std::thread t(task);


    std::thread t2(stats);
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

    unlink(fifoPath);

    return 0;
}

