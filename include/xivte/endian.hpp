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
// Windows Compatibility Mapping (Since Windows is strictly Little Endian)
#   define htole16(x) (x)
#   define htole32(x) (x)
#   define le32toh(x) (x)

#else
// Linux standard
#   include <endian.h>
#endif
