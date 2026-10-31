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
    if (fd == -1) {
        perror("open");
        _exit(1);
    }
    long s_number = 0;
    std::vector<int> payload(PAYLOAD_SIZE);
    p packet;
    for (;;) {

        if (!flag.load(std::memory_order_relaxed)) {
            continue;
        }

        fill_vector(payload, -42000, 42000); // типу йде звук

        packet.s_number = s_number;
        packet.crc = crc32(payload.data(), PAYLOAD_SIZE * sizeof(int));
        packet.timestamp = getTimestampSeconds();
        packet.payload = payload;

        auto data = serialize(packet, PAYLOAD_SIZE);

        if (!write_all(fd, data.data(), data.size())) {
            perror("write_all");
            break;
        }
        s_number++;

    }

}


int main(int argc, char* argv[]) {
    if (argc > 1) {
        int val = std::atoi(argv[1]);
        if (val > 0) {
            PAYLOAD_SIZE = val;
        } else {
            std::cerr << "Некоректний PAYLOAD_SIZE, використовується за замовчуванням: " << PAYLOAD_SIZE << "\n";
        }
    }
    std::cout << "PAYLOAD_SIZE = " << PAYLOAD_SIZE << "\n";

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
