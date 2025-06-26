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
        std::cerr << "PAPI error (" << msg << "): " << PAPI_strerror(code)
                  << "\n";
        std::exit(EXIT_FAILURE);
    }
}

// Measure cache hit rate (LLC) for a parallel read on a NUMA node
void measure_hit_rate(int numa_node)
{
    N = getenv_or("CACHE_TEST_SIZE", N);

    numa_set_preferred(numa_node);

    void* raw = nullptr;
    HPX_TEST_MSG(posix_memalign(&raw, 64, N * sizeof(int)) == 0,
        "posix_memalign failed");
    int* arr = static_cast<int*>(raw);

    for (std::size_t i = 0; i < N; ++i)
    {
        arr[i] = static_cast<int>(i);
    }

    int papi_ret = PAPI_library_init(PAPI_VER_CURRENT);
    if (papi_ret != PAPI_VER_CURRENT)
    {
        std::cerr << "PAPI library init error: " << PAPI_strerror(papi_ret)
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    int EventSet = PAPI_NULL;
    check_papi(PAPI_create_eventset(&EventSet), "create_eventset");
    check_papi(PAPI_add_event(EventSet, PAPI_L3_TCM), "add L3_TCM");
    check_papi(PAPI_add_event(EventSet, PAPI_L3_TCA), "add L3_TCA");

    long long counters[2] = {0, 0};
    check_papi(PAPI_start(EventSet), "start counters");

    auto t0 = std::chrono::high_resolution_clock::now();
    hpx::experimental::for_loop(
        hpx::execution::par, 0UL, N, [&](std::size_t i) {
            volatile int x = arr[i];
            (void) x;
        });
    auto t1 = std::chrono::high_resolution_clock::now();

    check_papi(PAPI_stop(EventSet, counters), "stop counters");
    long long llc_misses = counters[0];
    long long llc_accesses = counters[1];

    check_papi(PAPI_cleanup_eventset(EventSet), "cleanup_eventset");
    check_papi(PAPI_destroy_eventset(&EventSet), "destroy_eventset");

    free(arr);

    auto dur_us =
        std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    std::cout << "HPX Threads : " << n_threads << "\n";
    std::cout << "Node " << numa_node << ": L3_accesses=" << llc_accesses
              << ", L3_misses=" << llc_misses;

    double rate = 0.0;
    rate = double(llc_accesses - llc_misses) / double(llc_accesses);
    std::cout << ", hit_rate=" << (rate * 100.0) << "% "
              << ", time_us=" << dur_us << std::endl;
    if (!(llc_accesses > 0 && llc_misses >= 0 && llc_misses <= llc_accesses))
    {
        std::cerr << " [warning: invalid counter values]\n";
    }
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

    int papi_ret = PAPI_library_init(PAPI_VER_CURRENT);
    if (papi_ret != PAPI_VER_CURRENT)
    {
        std::cerr << "PAPI init error (std): " << PAPI_strerror(papi_ret)
                  << std::endl;
        std::exit(EXIT_FAILURE);
    }

    int EventSet = PAPI_NULL;
    check_papi(PAPI_create_eventset(&EventSet), "create_eventset");
    check_papi(PAPI_add_event(EventSet, PAPI_L3_TCM), "add L3_TCM");
    check_papi(PAPI_add_event(EventSet, PAPI_L3_TCA), "add L3_TCA");

    long long counters[2] = {0, 0};
    check_papi(PAPI_start(EventSet), "start counters");

    unsigned nthreads = n_threads;
    if (nthreads == 0)
        nthreads = 2;
    std::size_t chunk = N / nthreads;
    std::vector<std::thread> threads;
    threads.reserve(nthreads);

    auto t0 = std::chrono::high_resolution_clock::now();

    for (unsigned t = 0; t < nthreads; ++t)
    {
        std::size_t start = t * chunk;
        std::size_t end = (t + 1 == nthreads ? N : (t + 1) * chunk);
        threads.emplace_back([=]() {
            for (std::size_t i = start; i < end; ++i)
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
    long long misses = counters[0];
    long long accesses = counters[1];
    check_papi(PAPI_cleanup_eventset(EventSet), "cleanup_eventset");
    check_papi(PAPI_destroy_eventset(&EventSet), "destroy_eventset");
    free(arr);

    auto dur_us =
        std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    double rate = (accesses > 0 ? double(accesses - misses) / accesses : 0.0);
    rate = std::clamp(rate, 0.0, 1.0);
    std::cout << "std Threads : " << nthreads << "\n";
    std::cout << "[std] Node " << numa_node << ": access=" << accesses
              << ", miss=" << misses << ", hit=" << (rate * 100.) << "%"
              << ", time_us=" << dur_us << "\n";
}

int hpx_main(hpx::program_options::variables_map& vm)
{
    int max_node = numa_max_node();
    HPX_TEST_MSG(max_node + 1 >= 2, "Require >=2 NUMA nodes");
    n_threads = hpx::resource::get_num_threads();

    measure_hit_rate(0);
    measure_hit_rate(1);

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    HPX_TEST_EQ_MSG(hpx::local::init(hpx_main, argc, argv), 0,
        "HPX main exited with non-zero status");

    // std::thread measurement after HPX
    measure_hit_rate_std(0);
    measure_hit_rate_std(1);

    return hpx::util::report_errors();
}
