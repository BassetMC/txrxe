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

void task() {
    const char* fifoPath = "/dev/shm/txrxe1";
    int fd = open(fifoPath, O_WRONLY);
    if (fd == -1) {
        perror("open");
        _exit(1);
    }

    if (write(fd, &PAYLOAD_SIZE, (size_t)sizeof(int)) != sizeof(int)) {
        perror("write PAYLOAD_SIZE");
        _exit(1);
    }

    long s_number = 0;
    p packet(PAYLOAD_SIZE);
    std::vector<uint8_t> buf(get_packet_size(PAYLOAD_SIZE));
    for (;;) {

        if (!flag.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        fill_vector(packet.payload, -42000, 42000); // типу йде звук

        packet.s_number = s_number;
        packet.crc = crc32(packet.payload.data(), PAYLOAD_SIZE * sizeof(int));
        packet.timestamp = getTimestampSeconds();

        serialize(packet, PAYLOAD_SIZE, buf);

        if (!write_all(fd, buf.data(), buf.size())) {
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
            _exit(0);
        }
        else {
            flag = !flag;
            std::cout << "Натиснуто не ESC. flag зараз: " << flag <<"\n";
        }

    }

    t.join();
    return 0;
}
