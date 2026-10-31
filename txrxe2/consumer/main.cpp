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
#include "core/hash.h"

std::atomic<bool> flag{true};
std::atomic<uint64_t> total{0};

int PAYLOAD_SIZE = 4096;
const char* fifoPath = "/dev/shm/txrxe1";

void task() {


    unlink(fifoPath);
    mkfifo(fifoPath, 0600);
    int fd = open(fifoPath, O_RDONLY);
    if (fd == -1) {
        perror("open");
        _exit(1);
    }

    for (;;) {
        if (!flag.load(std::memory_order_relaxed)) {
            continue;
        }
        std::vector<uint8_t> buf(get_packet_size(PAYLOAD_SIZE));

        int res = read(fd, buf.data(), buf.size());
        if (res == buf.size()) {
            p packet = deserialize(buf.data(), buf.size(), PAYLOAD_SIZE);
            // std::cout << packet.crc << " " << crc32(packet.payload.data(), PAYLOAD_SIZE) << "\n";
            if (
                packet.crc == crc32(packet.payload.data(), PAYLOAD_SIZE * sizeof(int))
                )
            {
                total++;
            }


        }

    }

}

void stats() {
    uint64_t prev_total = 0;

    for (;;) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        uint64_t curr_total = total.load(std::memory_order_relaxed);
        uint64_t pps = curr_total - prev_total;
        uint64_t bps = pps * get_packet_size(PAYLOAD_SIZE);
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

