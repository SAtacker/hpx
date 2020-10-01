//  Copyright (c) 2014-2020 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/include/parallel_all_any_none_of.hpp>
#include <hpx/modules/testing.hpp>
#include <hpx/parallel/util/projection_identity.hpp>

#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "test_utils.hpp"

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_any_of(IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t iseq[] = {0, 23, 10007};
    for (std::size_t i : iseq)
    {
        std::vector<std::size_t> c =
            test::fill_all_any_none(10007, i);    //-V106

        bool result = hpx::any_of(iterator(std::begin(c)),
            iterator(std::end(c)), [](std::size_t v) { return v != 0; });

        // verify values
        bool expected = std::any_of(
            std::begin(c), std::end(c), [](std::size_t v) { return v != 0; });

        HPX_TEST_EQ(result, expected);
    }
}

template <typename ExPolicy, typename IteratorTag>
void test_any_of(ExPolicy&& policy, IteratorTag)
{
    static_assert(hpx::is_execution_policy<ExPolicy>::value,
        "hpx::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t iseq[] = {0, 23, 10007};
    for (std::size_t i : iseq)
    {
        std::vector<std::size_t> c =
            test::fill_all_any_none(10007, i);    //-V106

        bool result = hpx::any_of(policy, iterator(std::begin(c)),
            iterator(std::end(c)), [](std::size_t v) { return v != 0; });

        // verify values
        bool expected = std::any_of(
            std::begin(c), std::end(c), [](std::size_t v) { return v != 0; });

        HPX_TEST_EQ(result, expected);
    }
}

template <typename IteratorTag,
    typename Proj = hpx::parallel::util::projection_identity>
void test_any_of_ranges_seq(IteratorTag, Proj proj = Proj())
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t iseq[] = {0, 23, 10007};
    for (std::size_t i : iseq)
    {
        std::vector<std::size_t> c =
            test::fill_all_any_none(10007, i);    //-V106

        bool result = hpx::ranges::any_of(
            iterator(std::begin(c)), iterator(std::end(c)),
            [](std::size_t v) { return v != 0; }, proj);

        // verify values
        bool expected = std::any_of(std::begin(c), std::end(c),
            [proj](std::size_t v) { return proj(v) != 0; });

        HPX_TEST_EQ(result, expected);
    }
}

template <typename ExPolicy, typename IteratorTag,
    typename Proj = hpx::parallel::util::projection_identity>
void test_any_of_ranges(ExPolicy&& policy, IteratorTag, Proj proj = Proj())
{
    static_assert(hpx::is_execution_policy<ExPolicy>::value,
        "hpx::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t iseq[] = {0, 23, 10007};
    for (std::size_t i : iseq)
    {
        std::vector<std::size_t> c =
            test::fill_all_any_none(10007, i);    //-V106

        bool result = hpx::ranges::any_of(
            policy, iterator(std::begin(c)), iterator(std::end(c)),
            [](std::size_t v) { return v != 0; }, proj);

        // verify values
        bool expected = std::any_of(std::begin(c), std::end(c),
            [proj](std::size_t v) { return proj(v) != 0; });

        HPX_TEST_EQ(result, expected);
    }
}

template <typename ExPolicy, typename IteratorTag>
void test_any_of_async(ExPolicy&& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t iseq[] = {0, 23, 10007};
    for (std::size_t i : iseq)
    {
        std::vector<std::size_t> c =
            test::fill_all_any_none(10007, i);    //-V106

        hpx::future<bool> f = hpx::any_of(p, iterator(std::begin(c)),
            iterator(std::end(c)), [](std::size_t v) { return v != 0; });
        f.wait();

        // verify values
        bool expected = std::any_of(
            std::begin(c), std::end(c), [](std::size_t v) { return v != 0; });

        HPX_TEST_EQ(expected, f.get());
    }
}

template <typename ExPolicy, typename IteratorTag,
    typename Proj = hpx::parallel::util::projection_identity>
void test_any_of_ranges_async(ExPolicy&& p, IteratorTag, Proj proj = Proj())
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t iseq[] = {0, 23, 10007};
    for (std::size_t i : iseq)
    {
        std::vector<std::size_t> c =
            test::fill_all_any_none(10007, i);    //-V106

        hpx::future<bool> f = hpx::ranges::any_of(
            p, iterator(std::begin(c)), iterator(std::end(c)),
            [](std::size_t v) { return v != 0; }, proj);
        f.wait();

        // verify values
        bool expected = std::any_of(std::begin(c), std::end(c),
            [proj](std::size_t v) { return proj(v) != 0; });

        HPX_TEST_EQ(expected, f.get());
    }
}

template <typename IteratorTag>
void test_any_of()
{
    struct proj
    {
        //This projection should cause tests to fail if it is not applied
        //because it causes predicate to evaluate the opposite
        constexpr std::size_t operator()(std::size_t x) const
        {
            return !static_cast<bool>(x);
        }
    };
    using namespace hpx::execution;

    test_any_of(IteratorTag());
    test_any_of_ranges_seq(IteratorTag(), proj());

    test_any_of(seq, IteratorTag());
    test_any_of(par, IteratorTag());
    test_any_of(par_unseq, IteratorTag());

    test_any_of_ranges(seq, IteratorTag(), proj());
    test_any_of_ranges(par, IteratorTag(), proj());
    test_any_of_ranges(par_unseq, IteratorTag(), proj());

    test_any_of_async(seq(task), IteratorTag());
    test_any_of_async(par(task), IteratorTag());

    test_any_of_ranges_async(seq(task), IteratorTag(), proj());
    test_any_of_ranges_async(par(task), IteratorTag(), proj());
}

// template <typename IteratorTag>
// void test_any_of_exec()
// {
//     using namespace hpx::execution;
//
//     {
//         hpx::threads::executors::local_priority_queue_executor exec;
//         test_any_of(par(exec), IteratorTag());
//     }
//     {
//         hpx::threads::executors::local_priority_queue_executor exec;
//         test_any_of(task(exec), IteratorTag());
//     }
//
//     {
//         hpx::threads::executors::local_priority_queue_executor exec;
//         test_any_of(execution_policy(par(exec)), IteratorTag());
//     }
//     {
//         hpx::threads::executors::local_priority_queue_executor exec;
//         test_any_of(execution_policy(task(exec)), IteratorTag());
//     }
// }

void any_of_test()
{
    test_any_of<std::random_access_iterator_tag>();
    test_any_of<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_any_of_exception(IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t iseq[] = {0, 23, 10007};
    for (std::size_t i : iseq)
    {
        std::vector<std::size_t> c =
            test::fill_all_any_none(10007, i);    //-V106

        bool caught_exception = false;
        try
        {
            hpx::any_of(iterator(std::begin(c)), iterator(std::end(c)),
                [](std::size_t v) {
                    return throw std::runtime_error("test"), v != 0;
                });

            HPX_TEST(false);
        }
        catch (hpx::exception_list const& e)
        {
            caught_exception = true;
            test::test_num_exceptions<hpx::execution::sequenced_policy,
                IteratorTag>::call(hpx::execution::seq, e);
        }
        catch (...)
        {
            HPX_TEST(false);
        }

        HPX_TEST(caught_exception);
    }
}

template <typename ExPolicy, typename IteratorTag>
void test_any_of_exception(ExPolicy&& policy, IteratorTag)
{
    static_assert(hpx::is_execution_policy<ExPolicy>::value,
        "hpx::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t iseq[] = {0, 23, 10007};
    for (std::size_t i : iseq)
    {
        std::vector<std::size_t> c =
            test::fill_all_any_none(10007, i);    //-V106

        bool caught_exception = false;
        try
        {
            hpx::any_of(policy, iterator(std::begin(c)), iterator(std::end(c)),
                [](std::size_t v) {
                    return throw std::runtime_error("test"), v != 0;
                });

            HPX_TEST(false);
        }
        catch (hpx::exception_list const& e)
        {
            caught_exception = true;
            test::test_num_exceptions<ExPolicy, IteratorTag>::call(policy, e);
        }
        catch (...)
        {
            HPX_TEST(false);
        }

        HPX_TEST(caught_exception);
    }
}

template <typename ExPolicy, typename IteratorTag>
void test_any_of_exception_async(ExPolicy&& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t iseq[] = {0, 23, 10007};
    for (std::size_t i : iseq)
    {
        std::vector<std::size_t> c =
            test::fill_all_any_none(10007, i);    //-V106

        bool caught_exception = false;
        bool returned_from_algorithm = false;
        try
        {
            hpx::future<void> f = hpx::any_of(p, iterator(std::begin(c)),
                iterator(std::end(c)), [](std::size_t v) {
                    return throw std::runtime_error("test"), v != 0;
                });
            returned_from_algorithm = true;
            f.get();

            HPX_TEST(false);
        }
        catch (hpx::exception_list const& e)
        {
            caught_exception = true;
            test::test_num_exceptions<ExPolicy, IteratorTag>::call(p, e);
        }
        catch (...)
        {
            HPX_TEST(false);
        }

        HPX_TEST(caught_exception);
        HPX_TEST(returned_from_algorithm);
    }
}

template <typename IteratorTag>
void test_any_of_exception()
{
    using namespace hpx::execution;

    test_any_of_exception(IteratorTag());

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_any_of_exception(seq, IteratorTag());
    test_any_of_exception(par, IteratorTag());

    test_any_of_exception_async(seq(task), IteratorTag());
    test_any_of_exception_async(par(task), IteratorTag());
}

void any_of_exception_test()
{
    test_any_of_exception<std::random_access_iterator_tag>();
    test_any_of_exception<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename ExPolicy, typename IteratorTag>
void test_any_of_bad_alloc(ExPolicy&& policy, IteratorTag)
{
    static_assert(hpx::is_execution_policy<ExPolicy>::value,
        "hpx::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t iseq[] = {0, 23, 10007};
    for (std::size_t i : iseq)
    {
        std::vector<std::size_t> c =
            test::fill_all_any_none(10007, i);    //-V106

        bool caught_exception = false;
        try
        {
            hpx::any_of(policy, iterator(std::begin(c)), iterator(std::end(c)),
                [](std::size_t v) { return throw std::bad_alloc(), v != 0; });

            HPX_TEST(false);
        }
        catch (std::bad_alloc const&)
        {
            caught_exception = true;
        }
        catch (...)
        {
            HPX_TEST(false);
        }

        HPX_TEST(caught_exception);
    }
}

template <typename ExPolicy, typename IteratorTag>
void test_any_of_bad_alloc_async(ExPolicy&& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t iseq[] = {0, 23, 10007};
    for (std::size_t i : iseq)
    {
        std::vector<std::size_t> c =
            test::fill_all_any_none(10007, i);    //-V106

        bool caught_exception = false;
        bool returned_from_algorithm = false;
        try
        {
            hpx::future<void> f = hpx::any_of(p, iterator(std::begin(c)),
                iterator(std::end(c)),
                [](std::size_t v) { return throw std::bad_alloc(), v != 0; });
            returned_from_algorithm = true;
            f.get();

            HPX_TEST(false);
        }
        catch (std::bad_alloc const&)
        {
            caught_exception = true;
        }
        catch (...)
        {
            HPX_TEST(false);
        }

        HPX_TEST(caught_exception);
        HPX_TEST(returned_from_algorithm);
    }
}

template <typename IteratorTag>
void test_any_of_bad_alloc()
{
    using namespace hpx::execution;

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_any_of_bad_alloc(seq, IteratorTag());
    test_any_of_bad_alloc(par, IteratorTag());

    test_any_of_bad_alloc_async(seq(task), IteratorTag());
    test_any_of_bad_alloc_async(par(task), IteratorTag());
}

void any_of_bad_alloc_test()
{
    test_any_of_bad_alloc<std::random_access_iterator_tag>();
    test_any_of_bad_alloc<std::forward_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(hpx::program_options::variables_map& vm)
{
    any_of_test();
    any_of_exception_test();
    any_of_bad_alloc_test();
    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    // add command line option which controls the random number generator seed
    using namespace hpx::program_options;
    options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    // By default this test should run on all available cores
    std::vector<std::string> const cfg = {"hpx.os_threads=all"};

    // Initialize and run HPX
    HPX_TEST_EQ_MSG(hpx::init(desc_commandline, argc, argv, cfg), 0,
        "HPX main exited with non-zero status");

    return hpx::util::report_errors();
}