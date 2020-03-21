#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <fcntl.h>
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

bool check(const char *buffer, const char *target, ssize_t start, size_t target_size) {
    for (size_t i = 0; i < target_size; i++, start = (start + 1) % target_size) {
        if (buffer[start] != target[i]) {
            return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Please enter STRING and FILE NAME";
        return EXIT_FAILURE;
    }

    ssize_t target_size = strlen(argv[1]);
    uint32_t target_hash = hash(argv[1], target_size);

    FILE *fd = fopen(argv[2], "rb");
    if (fd == nullptr) {
        perror("file open failed");
        return EXIT_FAILURE;
    }

    size_t cnt = 0;
    uint32_t actual_hash = 0;

    uint32_t delta_power = pow(POWER, target_size - 1);

    char buffer[target_size];

    char c = getc(fd);
    for (; cnt < target_size && c != EOF; ++cnt, c = getc(fd)) {
        buffer[cnt] = c;
        actual_hash = actual_hash * POWER + c;
    }
    if (c == EOF && cnt < target_size) {
        std::cout << "false\n";
        fclose(fd);
        return EXIT_SUCCESS;
    }

    size_t i = 0;
    for (; c != EOF; c = getc(fd)) {
        if (actual_hash == target_hash) {
            bool res = check(buffer, argv[1], i, target_size);
            if (res) {
                std::cout << "true\n";
                fclose(fd);
                return EXIT_SUCCESS;
            }
        } else {
            actual_hash = actual_hash - buffer[i] * delta_power;
            actual_hash = actual_hash * POWER + c;

            buffer[i] = c;
            i = (i + 1) % target_size;
        }
    }

    std::cout << "false\n";
    fclose(fd);
    return EXIT_SUCCESS;
}
