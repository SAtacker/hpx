// Copyright (C) 2025 Shreyas Atre
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/algorithms.hpp>
#include <hpx/future.hpp>
#include <hpx/init.hpp>
#include <hpx/modules/testing.hpp>
#include <hpx/thread.hpp>

#include <chrono>
#include <exception>
#include <mutex>
#include <utility>
#include <vector>

#include <numa.h>
#include <papi.h>

#include <cstdlib>

// Default size: 10M ints (40MB)
static std::size_t N = 10 * 1024 * 1024;
static const std::size_t max_threads_parallel =
    std::thread::hardware_concurrency();
static std::size_t n_threads = std::thread::hardware_concurrency();

static std::size_t getenv_or(const char* var, std::size_t def)
{
    const char* val = std::getenv(var);
    return val ? std::stoull(val) : def;
}

static void check_papi(int code, const char* msg)
{
    if (code != PAPI_OK)
    {
        std::cerr << "PAPI error (" << msg << "): "
                  << "\n";
        // std::exit(EXIT_FAILURE);
    }
}

// Measure cache hit rates for L1, L2, and L3 on a parallel read
void measure_hit_rate(int numa_node)
{
    N = getenv_or("CACHE_TEST_SIZE", N);
    numa_set_preferred(numa_node);

    // Allocate aligned array
    void* raw = nullptr;
    HPX_TEST_MSG(posix_memalign(&raw, 64, N * sizeof(int)) == 0,
        "posix_memalign failed");
    int* arr = static_cast<int*>(raw);
    for (std::size_t i = 0; i < N; ++i)
        arr[i] = static_cast<int>(i);

    int EventSet = PAPI_NULL;
    check_papi(PAPI_create_eventset(&EventSet), "create_eventset");

    // Query and add events if available
    bool has_L1 = (PAPI_query_event(PAPI_L1_TCM) == PAPI_OK &&
        PAPI_query_event(PAPI_L1_TCA) == PAPI_OK);
    bool has_L2 = (PAPI_query_event(PAPI_L2_TCM) == PAPI_OK &&
        PAPI_query_event(PAPI_L2_TCA) == PAPI_OK);
    bool has_L3 = (PAPI_query_event(PAPI_L3_TCM) == PAPI_OK &&
        PAPI_query_event(PAPI_L3_TCA) == PAPI_OK);

    if (has_L1)
    {
        check_papi(PAPI_add_event(EventSet, PAPI_L1_TCM), "add L1_TCM");
        check_papi(PAPI_add_event(EventSet, PAPI_L1_TCA), "add L1_TCA");
    }

    if (has_L2)
    {
        check_papi(PAPI_add_event(EventSet, PAPI_L2_TCM), "add L2_TCM");
        check_papi(PAPI_add_event(EventSet, PAPI_L2_TCA), "add L2_TCA");
    }

    if (has_L3)
    {
        check_papi(PAPI_add_event(EventSet, PAPI_L3_TCM), "add L3_TCM");
        check_papi(PAPI_add_event(EventSet, PAPI_L3_TCA), "add L3_TCA");
    }

    // Allocate counters for up to 6 events
    long long counters[6] = {0};
    check_papi(PAPI_start(EventSet), "start counters");

    auto t0 = std::chrono::high_resolution_clock::now();
    hpx::experimental::for_loop(
        hpx::execution::par, 0UL, N, [&](std::size_t i) {
            volatile int x = arr[i];
            (void) x;
        });
    auto t1 = std::chrono::high_resolution_clock::now();

    check_papi(PAPI_stop(EventSet, counters), "stop counters");

    // Map counters: order matches add_event sequence
    size_t idx = 0;
    long long l1_miss = 0, l1_acc = 0, l2_miss = 0, l2_acc = 0, l3_miss = 0,
              l3_acc = 0;
    if (has_L1)
    {
        l1_miss = counters[idx++];
        l1_acc = counters[idx++];
    }
    if (has_L2)
    {
        l2_miss = counters[idx++];
        l2_acc = counters[idx++];
    }
    if (has_L3)
    {
        l3_miss = counters[idx++];
        l3_acc = counters[idx++];
    }

    check_papi(PAPI_cleanup_eventset(EventSet), "cleanup_eventset");
    check_papi(PAPI_destroy_eventset(&EventSet), "destroy_eventset");
    free(arr);

    auto dur_us =
        std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    std::cout << "HPX Threads: " << n_threads << " ";
    std::cout << "Node " << numa_node;
    if (has_L1)
        std::cout << ", L1_hit=" << (100.0 * (l1_acc - l1_miss) / l1_acc)
                  << "%";
    if (has_L2)
        std::cout << ", L2_hit=" << (100.0 * (l2_acc - l2_miss) / l2_acc)
                  << "%";
    if (has_L3)
        std::cout << ", L3_hit=" << (100.0 * (l3_acc - l3_miss) / l3_acc)
                  << "%";
    std::cout << ", time_us=" << dur_us << "\n";
}

// std::thread-based measurement
void measure_hit_rate_std(int numa_node)
{
    N = getenv_or("CACHE_TEST_SIZE", N);
    numa_set_preferred(numa_node);
    void* raw = nullptr;
    if (posix_memalign(&raw, 64, N * sizeof(int)) != 0)
    {
        std::cerr << "posix_memalign failed (std)" << std::endl;
        std::exit(EXIT_FAILURE);
    }
    int* arr = static_cast<int*>(raw);
    for (std::size_t i = 0; i < N; ++i)
        arr[i] = static_cast<int>(i);

    int EventSet = PAPI_NULL;
    check_papi(PAPI_create_eventset(&EventSet), "create_eventset");

    bool has_L1 = (PAPI_query_event(PAPI_L1_TCM) == PAPI_OK &&
        PAPI_query_event(PAPI_L1_TCA) == PAPI_OK);
    bool has_L2 = (PAPI_query_event(PAPI_L2_TCM) == PAPI_OK &&
        PAPI_query_event(PAPI_L2_TCA) == PAPI_OK);
    bool has_L3 = (PAPI_query_event(PAPI_L3_TCM) == PAPI_OK &&
        PAPI_query_event(PAPI_L3_TCA) == PAPI_OK);

    if (has_L1)
    {
        check_papi(PAPI_add_event(EventSet, PAPI_L1_TCM), "add L1_TCM");
        check_papi(PAPI_add_event(EventSet, PAPI_L1_TCA), "add L1_TCA");
    }
    if (has_L2)
    {
        check_papi(PAPI_add_event(EventSet, PAPI_L2_TCM), "add L2_TCM");
        check_papi(PAPI_add_event(EventSet, PAPI_L2_TCA), "add L2_TCA");
    }
    if (has_L3)
    {
        check_papi(PAPI_add_event(EventSet, PAPI_L3_TCM), "add L3_TCM");
        check_papi(PAPI_add_event(EventSet, PAPI_L3_TCA), "add L3_TCA");
    }

    long long counters[6] = {0};
    check_papi(PAPI_start(EventSet), "start counters");

    auto t0 = std::chrono::high_resolution_clock::now();
    unsigned nthr = n_threads > 0 ? n_threads : 2;
    std::size_t chunk = N / nthr;
    std::vector<std::thread> threads;
    threads.reserve(nthr);
    for (unsigned t = 0; t < nthr; ++t)
    {
        std::size_t s = t * chunk;
        std::size_t e = (t + 1 == nthr ? N : (t + 1) * chunk);
        threads.emplace_back([=]() {
            for (std::size_t i = s; i < e; ++i)
            {
                volatile int x = arr[i];
                (void) x;
            }
        });
    }
    for (auto& th : threads)
        th.join();
    auto t1 = std::chrono::high_resolution_clock::now();

    check_papi(PAPI_stop(EventSet, counters), "stop counters");

    size_t idx = 0;
    long long l1_m = 0, l1_a = 0, l2_m = 0, l2_a = 0, l3_m = 0, l3_a = 0;
    if (has_L1)
    {
        l1_m = counters[idx];
        l1_a = counters[idx + 1];
        idx += 2;
    }
    if (has_L2)
    {
        l2_m = counters[idx];
        l2_a = counters[idx + 1];
        idx += 2;
    }
    if (has_L3)
    {
        l3_m = counters[idx];
        l3_a = counters[idx + 1];
        idx += 2;
    }

    check_papi(PAPI_cleanup_eventset(EventSet), "cleanup_eventset");
    check_papi(PAPI_destroy_eventset(&EventSet), "destroy_eventset");
    free(arr);

    auto dur_us =
        std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "std Threads: " << nthr;
    std::cout << " Node " << numa_node;
    if (has_L1)
        std::cout << ", L1_hit=" << (100.0 * (l1_a - l1_m) / l1_a) << "%";
    if (has_L2)
        std::cout << ", L2_hit=" << (100.0 * (l2_a - l2_m) / l2_a) << "%";
    if (has_L3)
        std::cout << ", L3_hit=" << (100.0 * (l3_a - l3_m) / l3_a) << "%";
    std::cout << ", time_us=" << dur_us << "\n";
}

int hpx_main(hpx::program_options::variables_map& vm)
{
    int max_node = numa_max_node();
    HPX_TEST_MSG(max_node + 1 >= 2, "Require >=2 NUMA nodes");
    for (n_threads = 1; n_threads < max_threads_parallel + 1; n_threads++)
    {
        measure_hit_rate(0);
        measure_hit_rate(1);
    }

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // Init PAPI
    check_papi(PAPI_library_init(PAPI_VER_CURRENT), "library_init");
    for (n_threads = 1; n_threads < max_threads_parallel + 1; n_threads++)
    {
        measure_hit_rate_std(0);
        measure_hit_rate_std(1);
    }

    HPX_TEST_EQ_MSG(hpx::local::init(hpx_main, argc, argv), 0,
        "HPX main exited with non-zero status");

    // std::thread measurement after HPX

    return hpx::util::report_errors();
}
