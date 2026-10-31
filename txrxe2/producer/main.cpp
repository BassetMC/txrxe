#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <vector>
#include <atomic>
#include <thread>
#include "core/txrx.h"
#include "core/hash.h"
#include <ctime>

std::atomic<bool> flag{true};

int PAYLOAD_SIZE = 4096;
const char* fifoPath = "/dev/shm/txrxe1";

void task() {
    int fd = open(fifoPath, O_WRONLY);
    long s_number = 0;
    for (;;) {

        if (!flag.load(std::memory_order_relaxed)) {
            continue;
        }

        std::vector<int> payload(PAYLOAD_SIZE);

        fill_vector(payload, -42000, 42000); // типу йде звук

        p packet;

        packet.s_number = s_number;
        packet.crc = crc32(payload.data(), PAYLOAD_SIZE * sizeof(int));
        packet.timestamp = getTimestampSeconds();
        packet.payload = payload;


        if (fd == -1) {
            perror("open");
            _exit(1);
        }

        auto data = serialize(packet, PAYLOAD_SIZE);

        ssize_t n = write(fd, data.data(), data.size());
        s_number++;
        if (n == -1) {
            perror("write");
        }

        // close(fd);
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
