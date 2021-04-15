#include <iostream>
#include <fstream>

#include "SpeedTest.h"

#define XXH_INLINE_ALL

#include "xxhash.h"
#include "MurmurHash3.h"

int trials = 100000;

template<typename T>
std::function<double(int)> wrap(SpeedTest::hash_seed<T> hasher) {
    SpeedTestSeed tester{hasher, trials};
    return [tester](int key_size) {
        return tester.run(key_size);
    };
}

template<typename T>
std::function<double(int)> wrap(SpeedTest::hash_no_seed<T> hasher) {
    SpeedTestNoSeed tester{hasher, trials};
    return [tester](int key_size) {
        return tester.run(key_size);
    };
}

void run(int from, int to, int step, const std::string &filename) {
    std::ofstream output{filename};

    output << "bytes";

    std::vector<std::function<double(int)>> hashers;
/*
    output << ",MurmurHash3 x86 32";
    hashers.emplace_back(wrap(MurmurHash3_x86_32));

    output << ",MurmurHash3 x86 128";
    hashers.emplace_back(wrap(MurmurHash3_x86_128));

    output << ",MurmurHash3 x64 128";
    hashers.emplace_back(wrap(MurmurHash3_x64_128));
*/
    output << ",XXH3 128bit";
    hashers.emplace_back(wrap(XXH3_128bits));

    output << ",XXH3 64bit";
    hashers.emplace_back(wrap(XXH3_64bits));

    output << '\n';

    while (from <= to) {
        output << from;
        for (auto &hasher : hashers) {
            output << ',' << hasher(from);
        }
        output << '\n';
        from += step;
    }

    output.close();
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        std::cout << "Give me three integers, please" << std::endl;
    } else {
        int from = atoi(argv[1]), to = atoi(argv[2]), step = atoi(argv[3]);
        if (to < from || step <= 0) {
            std::cout << "Your integers are incorrect" << std::endl;
        } else {
            std::string output_filename;
            if (argc >= 5) {
                output_filename = argv[4];
            } else {
                output_filename = "out.csv";
            }
            if (argc >= 6) {
                trials = atoi(argv[5]);
            }
            run(from, to, step, output_filename);
        }
    }
}
