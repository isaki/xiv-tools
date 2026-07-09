/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2026 isaki */

#pragma once

#include <cstdint>

namespace isaki::xivte
{

    //
    // DDS Support
    //
    inline constexpr char DDS_MAGIC[] = { 'D', 'D', 'S', ' ' };

    // Pulled from M$'s documentation

    // These are DDS_PIXELFORMAT.fourCC values
    inline constexpr std::uint32_t DDS_DXT1 = 0x31545844;    // BC1
    inline constexpr std::uint32_t DDS_DXT5 = 0x35545844;    // BC3
    inline constexpr std::uint32_t DDS_DX10 = 0x30315844;    // DX10 (BC5, 7)

    // DXGI Enum values
    enum class DXGI_FORMAT : std::uint32_t
    {
        // 8888
        R8G8B8A8_UNORM = 0x1C,
        B8G8R8A8_UNORM = 0x57,

        // Compressed
        BC1_UNORM = 0x47,
        BC3_UNORM = 0x77,
        BC5_UNORM = 0x53,
        BC7_UNORM = 0x62
    };

    //
    // DDS/DXGI Structs (C Style)
    // Pease note these are 1 byte aligned
    //

#pragma pack(push, 1)
    struct DDS_PIXELFORMAT {
        std::uint32_t size;
        std::uint32_t flags;
        std::uint32_t fourCC;
        std::uint32_t rgbBitCount;
        std::uint32_t rBitMask;
        std::uint32_t gBitMask;
        std::uint32_t bBitMask;
        std::uint32_t aBitMask;
    };

    struct DDS_HEADER {
        std::uint32_t size;
        std::uint32_t flags;
        std::uint32_t height;
        std::uint32_t width;
        std::uint32_t pitchOrLinearSize;
        std::uint32_t depth;
        std::uint32_t mipmapCount;
        std::uint32_t reserved1[11];
        DDS_PIXELFORMAT ddspf;
        std::uint32_t caps;
        std::uint32_t caps2;
        std::uint32_t caps3;
        std::uint32_t caps4;
        std::uint32_t reserved2;
    };

    struct DDS_HEADER_DXT10 {
        std::uint32_t dxgiFormat;
        std::uint32_t resourceDimension;
        std::uint32_t miscFlag;
        std::uint32_t arraySize;
        std::uint32_t miscFlags2;
    };
#pragma pack(pop)
}

