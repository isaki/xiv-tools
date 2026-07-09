/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2026 isaki */

#pragma once

#if defined(__APPLE__)
// macOS Compatibility Mapping
#   include <libkern/OSByteOrder.h>

#   define htole16(x) OSSwapHostToLittleInt16(x)
#   define htole32(x) OSSwapHostToLittleInt32(x)
#   define le32toh(x) OSSwapLittleToHostInt32(x)

#elif defined(_WIN32) || defined(_WIN64)
#   include <type_traits>
// Windows Compatibility Mapping (Since Windows is strictly Little Endian)
namespace isaki::xivte::internal
{
    template<typename X> requires std::is_integral_v<X>
    inline X win_endian_passthrough(X val) noexcept { return val; }
}

#   define htole16(x) isaki::xivte::internal::win_endian_passthrough<std::uint16_t>(x)
#   define htole32(x) isaki::xivte::internal::win_endian_passthrough<std::uint32_t>(x)
#   define le32toh(x) isaki::xivte::internal::win_endian_passthrough<std::uint32_t>(x)

#else
// Linux standard
#   include <endian.h>
#endif
