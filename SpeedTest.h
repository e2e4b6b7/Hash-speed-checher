#pragma once

#include <cstdint>
#include <memory.h>
#include <algorithm>
#include <numeric>
#include <cmath>

#include "SpeedTest.h"
#include "Random.h"

#define    FORCE_INLINE inline __attribute__((always_inline))
#define    NEVER_INLINE __attribute__((noinline))


class SpeedTest {
public:
    template<typename T>
    using hash_seed = T (*)(const void *blob, size_t len, uint32_t seed);

    template<typename T>
    using hash_no_seed = T (*)(const void *blob, size_t len);

    [[nodiscard]]
    double run(int key_size) const {
        Rand r(clock());

        auto *key = new uint8_t[key_size + 512];

        std::vector<uint64_t> times;
        times.reserve(trials);

        for (int i = 0; i < trials; i++) {
            r.rand_p(key, key_size);
            times.push_back(time_hash(key, key_size, i));
        }

        filter_outliers(times);

        delete[] key;

        return static_cast<double>(std::accumulate(times.begin(), times.end(), static_cast<uint64_t>(0)))
               / static_cast<double>(times.size());
    }

    virtual ~SpeedTest() = default;

protected:
    explicit SpeedTest(int trials) : trials(trials) {}

    static FORCE_INLINE uint64_t time_stamp() {
        unsigned int a, d;
        __asm__ volatile ("rdtsc" : "=a" (a), "=d" (d));
        return static_cast<uint64_t>(a) | (static_cast<uint64_t>(d) << 32);
    }

private:
    static bool contains_outlier(const std::vector<uint64_t> &v, long len) {
        double mean = static_cast<double>(std::accumulate(v.begin(), v.begin() + len, static_cast<uint64_t>(0)))
                      / static_cast<double>(len);
        double std_dev =
                std::accumulate(v.begin(), v.begin() + len, 0.0,
                                [mean](double sum, double val) { return sum + (val - mean) * (val - mean); })
                / static_cast<double>(len);
        return static_cast<double>(v[len - 1]) > mean + std::sqrt(std_dev) * 3;
    }

    static void filter_outliers(std::vector<uint64_t> &v) {
        std::sort(v.begin(), v.end());

        size_t len = 0;
        for (size_t x = 0x40000000; x; x = x >> 1) {
            if ((len | x) >= v.size()) continue;

            if (!contains_outlier(v, static_cast<long>(len | x))) {
                len |= x;
            }
        }
        v.resize(len);
    }

    virtual uint64_t time_hash(const void *key, int len, int seed) const = 0;

    const int trials;
};

template<typename T>
class SpeedTestSeed : public SpeedTest {
private:
    using hash = hash_seed<T>;

public:
    SpeedTestSeed(hash hasher, int trials) : SpeedTest(trials), hasher(hasher) {}

    ~SpeedTestSeed() override = default;

private:
    NEVER_INLINE uint64_t time_hash(const void *key, int len, int seed) const override {
        volatile uint64_t begin, end;

        begin = time_stamp();
        T hash_val = hasher(key, len, seed);
        end = time_stamp();

        return end - begin;
    }

    const hash hasher;
};

template<typename T>
class SpeedTestNoSeed : public SpeedTest {
private:
    using hash = hash_no_seed<T>;

public:
    SpeedTestNoSeed(hash hasher, int trials) : SpeedTest(trials), hasher(hasher) {}

    ~SpeedTestNoSeed() override = default;

private:
    NEVER_INLINE uint64_t time_hash(const void *key, int len, int) const override {
        volatile uint64_t begin, end;

        begin = time_stamp();
        T hash_val = hasher(key, len);
        end = time_stamp();

        return end - begin;
    }

    const hash hasher;
};
