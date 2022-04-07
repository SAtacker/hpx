//  Copyright (c) 2022 Shreyas Atre
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// This is a purely local version demonstrating the proposed extension to
// C++ implementing resumable functions (see N3564,
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3564.pdf). The
// necessary transformations are performed by hand.

#include <hpx/local/chrono.hpp>
#include <hpx/local/functional.hpp>
#include <hpx/local/future.hpp>
#include <hpx/local/init.hpp>

#include <hpx/future.hpp>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

///////////////////////////////////////////////////////////////////////////////
std::uint64_t threshold = 2;

///////////////////////////////////////////////////////////////////////////////
HPX_NOINLINE std::uint64_t fibonacci_serial(std::uint64_t n)
{
    if (n < 2)
        return n;
    return fibonacci_serial(n - 1) + fibonacci_serial(n - 2);
}

///////////////////////////////////////////////////////////////////////////////
//

hpx::future<std::uint64_t> fibonacci(std::uint64_t n)
{
    if (n <= 2)
        co_return 1;

    auto a = co_await fibonacci(n - 1);
    auto b = co_await fibonacci(n - 2);

    // iterate computing fib(n)
    co_return a + b;
}
//

hpx::future<std::uint64_t> fibonacci(std::uint64_t n);

///////////////////////////////////////////////////////////////////////////////
int hpx_main(hpx::program_options::variables_map& vm)
{
    // extract command line argument, i.e. fib(N)
    std::uint64_t n = vm["n-value"].as<std::uint64_t>();
    std::string test = vm["test"].as<std::string>();
    std::uint64_t max_runs = vm["n-runs"].as<std::uint64_t>();

    if (max_runs == 0)
    {
        std::cerr << "fibonacci_await: wrong command line argument value for "
                     "option 'n-runs', should not be zero"
                  << std::endl;
        return hpx::local::finalize();    // Handles HPX shutdown
    }

    threshold = vm["threshold"].as<unsigned int>();
    if (threshold < 2 || threshold > n)
    {
        std::cerr << "fibonacci_await: wrong command line argument value for "
                     "option 'threshold', should be in between 2 and n-value"
                     ", value specified: "
                  << threshold << std::endl;
        return hpx::local::finalize();    // Handles HPX shutdown
    }

    bool executed_one = false;
    std::uint64_t r = 0;

    if (test == "all" || test == "0")
    {
        // Keep track of the time required to execute.
        std::uint64_t start = hpx::chrono::high_resolution_clock::now();

        for (std::size_t i = 0; i != max_runs; ++i)
        {
            // serial execution
            r = fibonacci_serial(n);
        }

        // double d = double(hpx::chrono::high_resolution_clock::now() - start) / 1.e9;
        std::uint64_t d = hpx::chrono::high_resolution_clock::now() - start;
        char const* fmt = "fibonacci_serial({1}) == {2},"
                          "elapsed time:,{3},[s]\n";
        hpx::util::format_to(std::cout, fmt, n, r, d / max_runs);

        executed_one = true;
    }

    if (test == "all" || test == "1")
    {
        // Keep track of the time required to execute.
        std::uint64_t start = hpx::chrono::high_resolution_clock::now();

        for (std::size_t i = 0; i != max_runs; ++i)
        {
            // Create a future for the whole calculation, execute it locally,
            // and wait for it.
            r = fibonacci(n).get();
        }

        // double d = double(hpx::chrono::high_resolution_clock::now() - start) / 1.e9;
        std::uint64_t d = hpx::chrono::high_resolution_clock::now() - start;
        char const* fmt = "fibonacci_await({1}) == {2},"
                          "elapsed time:,{3},[s]\n";
        hpx::util::format_to(std::cout, fmt, n, r, d / max_runs);

        executed_one = true;
    }

    if (!executed_one)
    {
        std::cerr << "fibonacci_await: wrong command line argument value for "
                     "option 'tests', should be either 'all' or a number "
                     "between zero "
                     "and 1, value specified: "
                  << test << std::endl;
    }

    return hpx::local::finalize();    // Handles HPX shutdown
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    // Configure application-specific options
    hpx::program_options::options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    using hpx::program_options::value;
    // clang-format off
    desc_commandline.add_options()
        ("n-value", value<std::uint64_t>()->default_value(10),
         "n value for the Fibonacci function")
        ("n-runs", value<std::uint64_t>()->default_value(1),
         "number of runs to perform")
        ("threshold", value<unsigned int>()->default_value(2),
         "threshold for switching to serial code")
        ("test", value<std::string>()->default_value("all"),
        "select tests to execute (0-1, default: all)");
    // clang-format on

    // Initialize and run HPX
    hpx::local::init_params init_args;
    init_args.desc_cmdline = desc_commandline;

    return hpx::local::init(hpx_main, argc, argv, init_args);
}
