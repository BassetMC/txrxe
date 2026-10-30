
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>

int main() {
    const char* fifoPath = "/dev/shm/txrxe1";

    // Відкриваємо ТІЛЬКИ на запис (заблокується, поки читач не викличе open)
    int fd = open(fifoPath, O_WRONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    for (;;) {
        const char* msg = "HelloFIFO!";
        write(fd, msg, strlen(msg));
    }
    close(fd); // Надсилає EOF читачу
    return 0;
}