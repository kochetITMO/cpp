#include <cstdlib>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

const uint32_t POWER = 31;

uint32_t hash(const char *str, size_t size) {
    uint32_t result = 0;
    for (size_t i = 0; i < size; i++) {
        result = result * POWER + (uint32_t) str[i];
    }
    return result;
}

uint32_t pow(uint32_t x, uint32_t p) {
    uint32_t res = 1;
    while (p--) {
        res *= x;
    }
    return res;
}

bool check(
        const char *pred_buffer,
        const char *buffer,
        size_t BUFFER_SIZE,
        const char *target,
        size_t target_size,
        ssize_t start
) {
    size_t i = 0;
    while (start < 0 && i < target_size) {
        if (target[i] != pred_buffer[BUFFER_SIZE + start]) {
            return false;
        }
        i++;
        start++;
    }

    while (i < target_size) {
        if (target[i] != buffer[start]) {
            return false;
        }
        i++;
        start++;
    }

    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Please enter the STRING and FILE NAME";
        return EXIT_FAILURE;
    }

    size_t size = strlen(argv[1]);
    uint32_t target_hash = hash(argv[1], size);

    FILE *fd = fopen(argv[2], "rb");
    if (fd == nullptr) {
        perror("file open failed");
        return EXIT_FAILURE;
    }

    size_t cnt = 0;
    uint32_t actual_hash = 0;

    uint32_t delta_power = pow(POWER, size - 1);

    const size_t BUFFER_SIZE = 3;
    char pred_buffer[BUFFER_SIZE];

    for (;;) {
        char buffer[BUFFER_SIZE];
        ssize_t bytes_read = fread(buffer, sizeof(char), BUFFER_SIZE, fd);

        if (bytes_read < 0) {
            perror("read failed");
            fclose(fd);
            return EXIT_FAILURE;
        }

        if (bytes_read == 0) {
            std::cout << "false\n";
            break;
        }

        ssize_t i = 0;
        for (; cnt < size && i < bytes_read; ++i, ++cnt) {
            actual_hash = actual_hash * POWER + buffer[i];
        }
        if (i == bytes_read && cnt < size) {
            continue;
        }

        for (; i <= bytes_read; ++i) {
            if (actual_hash == target_hash) {
                bool res = check(
                        pred_buffer,
                        buffer,
                        BUFFER_SIZE,
                        argv[1],
                        size,
                        i - size
                );
                if (res) {
                    std::cout << "true\n";
                    fclose(fd);
                    return EXIT_SUCCESS;
                }
            } else if (i != bytes_read) {
                auto pred = BUFFER_SIZE + i - (ssize_t) size;
                auto now = i - (ssize_t) size;
                auto pred_char = (now >= 0) ?
                                 buffer[now] :
                                 pred_buffer[pred];
                auto add_char = buffer[i];

                actual_hash = actual_hash - pred_char * delta_power;
                actual_hash = actual_hash * POWER + add_char;
            }
        }

        std::memcpy(pred_buffer, buffer, bytes_read);
    }

    fclose(fd);
    return EXIT_SUCCESS;
}
