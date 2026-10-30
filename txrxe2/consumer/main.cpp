#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>
#include <unistd.h>
#include <atomic>


std::atomic<bool> flag{true};
std::atomic<long> total{0};
std::atomic<int> pps{0};
std::atomic<int> bps{0};



void task() {

    const char* fifoPath = "/dev/shm/txrxe1";
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
        char buf[5] = {0};
        read(fd, buf, 4);
        total++;
        // std::cout << "З FIFO: " << buf << '\n';

    }

}


int main() {


    std::thread t(task);
    t.detach();

    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        std::cout << "total: " << total.load() << '\n'
                  << "pps:   " << pps.exchange(0) << '\n'
                  << "bps:   " << bps.exchange(0) << "\n\n"
                  << std::flush;
    }

    return 0;
}
