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

    if (read(fd, &PAYLOAD_SIZE, sizeof(int)) != sizeof(int)) {
        perror("read PAYLOAD_SIZE");
        _exit(1);
    }

    std::cout << "PAYLOAD_SIZE = " << PAYLOAD_SIZE << "\n";
    std::vector<uint8_t> buf(get_packet_size(PAYLOAD_SIZE));

    for (;;) {
        if (!flag.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        if (!read_all(fd, buf.data(), buf.size())) {
            break;
        }

        p packet = deserialize(buf.data(), buf.size(), PAYLOAD_SIZE);

        if (
            packet.crc == crc32(packet.payload.data(), PAYLOAD_SIZE * sizeof(int))
            && packet.s_number == total.load()
            &&  packet.timestamp <= getTimestampSeconds() // перевірка чи пакет не прийшов з майбутнього
            )
        {
            total++;
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


int main(int argc, char* argv[]) {

    std::thread t(task);

    std::thread t2(stats);
    for (;;) {
        int key = getch_unix();

        if (key == 27) { // 27 — ASCII-код клавіші ESC
            std::cout << "Натиснуто ESC. Вихід з програми.\n";
            unlink(fifoPath);
            _exit(0);
        }
        else {
            flag = !flag;
            std::cout << "Натиснуто не ESC. flag зараз: " << flag <<"\n";
        }
    }

    return 0;
}

