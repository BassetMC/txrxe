
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <vector>
int PACKET_SIZE = 4096;
int main() {
    const char* fifoPath = "/dev/shm/txrxe1";

    for (;;) {


        int fd = open(fifoPath, O_WRONLY);
        if (fd == -1) {
            perror("open");
            return 1;
        }

        std::vector<char> msg(PACKET_SIZE, 'A');

        ssize_t n = write(fd, msg.data(), msg.size());
        if (n == -1) {
            perror("write");
        }

        close(fd);
    }

    return 0;
}