//  Copyright (c) 2018 Thomas Heller
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

/// \file hpx/coroutines/thread_id_type.hpp

#ifndef HPX_THREADS_THREAD_ID_TYPE_HPP
#define HPX_THREADS_THREAD_ID_TYPE_HPP

#include <hpx/config/constexpr.hpp>
#include <hpx/config/export_definitions.hpp>

#include <cstddef>
#include <functional>
#include <iosfwd>

namespace hpx { namespace threads {
    struct invalid_thread_id_tag
    {
    };

    HPX_CONSTEXPR_OR_CONST invalid_thread_id_tag invalid_thread_id;

    template <typename T>
    struct thread_id
    {
        constexpr thread_id()
          : thrd_(nullptr)
        {
        }
        constexpr thread_id(invalid_thread_id_tag)
          : thrd_(nullptr)
        {
        }
        explicit constexpr thread_id(T* thrd)
          : thrd_(thrd)
        {
        }

        thread_id(thread_id const&) = default;
        thread_id& operator=(thread_id const&) = default;

        constexpr T* operator->() const
        {
            return thrd_;
        }

        constexpr T& operator*() const
        {
            return *thrd_;
        }

        explicit constexpr operator bool() const
        {
            return nullptr != thrd_;
        }

        constexpr T* get() const
        {
            return thrd_;
        }

        HPX_CXX14_CONSTEXPR void reset()
        {
            thrd_ = nullptr;
        }

        friend constexpr bool operator==(std::nullptr_t, thread_id const& rhs)
        {
            return nullptr == rhs.thrd_;
        }

        friend constexpr bool operator!=(std::nullptr_t, thread_id const& rhs)
        {
            return nullptr != rhs.thrd_;
        }

        friend constexpr bool operator==(thread_id const& lhs, std::nullptr_t)
        {
            return nullptr == lhs.thrd_;
        }

        friend constexpr bool operator!=(thread_id const& lhs, std::nullptr_t)
        {
            return nullptr != lhs.thrd_;
        }

        friend constexpr bool operator==(
            invalid_thread_id_tag, thread_id const& rhs)
        {
            return nullptr == rhs.thrd_;
        }

        friend constexpr bool operator!=(
            invalid_thread_id_tag, thread_id const& rhs)
        {
            return nullptr != rhs.thrd_;
        }

        friend constexpr bool operator==(
            thread_id const& lhs, invalid_thread_id_tag)
        {
            return nullptr == lhs.thrd_;
        }

        friend constexpr bool operator!=(
            thread_id const& lhs, invalid_thread_id_tag)
        {
            return nullptr != lhs.thrd_;
        }

        friend constexpr bool operator==(
            thread_id const& lhs, thread_id const& rhs)
        {
            return lhs.thrd_ == rhs.thrd_;
        }

        friend constexpr bool operator!=(
            thread_id const& lhs, thread_id const& rhs)
        {
            return lhs.thrd_ != rhs.thrd_;
        }

        friend HPX_CXX14_CONSTEXPR bool operator<(
            thread_id const& lhs, thread_id const& rhs)
        {
            return std::less<void const*>{}(lhs.thrd_, rhs.thrd_);
        }

        friend HPX_CXX14_CONSTEXPR bool operator>(
            thread_id const& lhs, thread_id const& rhs)
        {
            return std::less<void const*>{}(rhs.thrd_, lhs.thrd_);
        }

        friend HPX_CXX14_CONSTEXPR bool operator<=(
            thread_id const& lhs, thread_id const& rhs)
        {
            return !(rhs > lhs);
        }

        friend HPX_CXX14_CONSTEXPR bool operator>=(
            thread_id const& lhs, thread_id const& rhs)
        {
            return !(rhs < lhs);
        }

        template <typename Char, typename Traits>
        friend std::basic_ostream<Char, Traits>& operator<<(
            std::basic_ostream<Char, Traits>& os, thread_id const& id)
        {
            os << id.get();
            return os;
        }

    private:
        T* thrd_;
    };
}}    // namespace hpx::threads

#endif
