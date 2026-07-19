/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2026 isaki */

#pragma once

#include <cstring>  // std::memcpy for floating point swaps
#include <cstdint>  // buffers for byte swaps
#include <bit>      // Compile time checks for endianness without macros
#include <concepts>
#include <type_traits>

#if defined(__APPLE__)
// macOS Compatibility Mapping
#   include <libkern/OSByteOrder.h>

#   define htole16(x) OSSwapHostToLittleInt16(x)
#   define htole32(x) OSSwapHostToLittleInt32(x)
#   define htole64(x) OSSwapHostToLittleInt64(x)
#   define le16toh(x) OSSwapLittleToHostInt16(x)
#   define le32toh(x) OSSwapLittleToHostInt32(x)
#   define le64toh(x) OSSwapLittleToHostInt64(x)

#elif defined(_WIN32) || defined(_WIN64)
// Windows Compatibility Mapping (Since Windows is strictly Little Endian)
namespace isaki::xiv::internal
{
    template<typename X> requires std::is_integral_v<X>
    inline X win_endian_passthrough(X val) noexcept { return val; }
}

#   define htole16(x) isaki::xiv::internal::win_endian_passthrough<std::uint16_t>(x)
#   define htole32(x) isaki::xiv::internal::win_endian_passthrough<std::uint32_t>(x)
#   define htole64(x) isaki::xiv::internal::win_endian_passthrough<std::uint64_t>(x)
#   define le16toh(x) isaki::xiv::internal::win_endian_passthrough<std::uint16_t>(x)
#   define le32toh(x) isaki::xiv::internal::win_endian_passthrough<std::uint32_t>(x)
#   define le64toh(x) isaki::xiv::internal::win_endian_passthrough<std::uint64_t>(x)

#else
// Linux standard
#   include <endian.h>
#endif

//
// Floating point handling
//

namespace isaki::xiv::internal
{
    template<typename X>
    requires std::is_floating_point_v<X>
    using MatchingInt_t = std::conditional_t<sizeof(X) == sizeof(std::uint32_t), std::uint32_t,
        std::conditional_t<sizeof(X) == sizeof(std::uint64_t), std::uint64_t, void>>;

    template<typename F, typename X>
    concept EndianSwap = std::is_floating_point_v<X> && requires(F f, MatchingInt_t<X> val)
    {
        { f(val) } -> std::same_as<MatchingInt_t<X>>;
    };

    template<typename X, typename F>
    requires EndianSwap<F, X>
    inline X swap(X val, F&& byteswap) noexcept
    {
        // Copy raw bits into an int
        MatchingInt_t<X> tmp;
        std::memcpy(&tmp, &val, sizeof(X));

        // Swap the bits as required
        tmp = byteswap(tmp);

        // Shove the bits back into the float
        std::memcpy(&val, &tmp, sizeof(X));

        // Return the result
        return val;
    }

    inline float swap_htole32f(float x) noexcept
    {
        if constexpr (std::endian::native == std::endian::big)
        {
            return swap<float>(x, [](MatchingInt_t<float> v) { return htole32(v); });
        }
        else
        {
            return x;
        }
    }

    inline float swap_le32ftoh(float x) noexcept
    {
        if constexpr (std::endian::native == std::endian::big)
        {
            return swap<float>(x, [](MatchingInt_t<float> v) { return le32toh(v); });
        }
        else
        {
            return x;
        }
    }

    inline double swap_htole64f(double x) noexcept
    {
        if constexpr (std::endian::native == std::endian::big)
        {
            return swap<double>(x, [](MatchingInt_t<double> v) { return htole64(v); });
        }
        else
        {
            return x;
        }
    }

    inline double swap_le64ftoh(double x) noexcept
    {
        if constexpr (std::endian::native == std::endian::big)
        {
            return swap<double>(x, [](MatchingInt_t<double> v) { return le64toh(v); });
        }
        else
        {
            return x;
        }
    }
}

#define htole32f(x) isaki::xiv::internal::swap_htole32f(x)
#define htole64f(x) isaki::xiv::internal::swap_htole64f(x)
#define le32ftoh(x) isaki::xiv::internal::swap_le32ftoh(x)
#define le64ftoh(x) isaki::xiv::internal::swap_le64ftoh(x)
