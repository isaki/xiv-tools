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
    // We do NOT care about endianness (though these are 32bit LE)
    // Reason being we write and read them as they come in via char
    // buffers, as long as we don't do math on them or anything, we
    // can treat them as opaque blocks of data.

    // These are DDS_PIXELFORMAT.fourCC values
    inline constexpr uint32_t DDS_DXT1 = 0x31545844;    // BC1
    inline constexpr uint32_t DDS_DXT5 = 0x35545844;    // BC3
    inline constexpr uint32_t DDS_DX10 = 0x30315844;    // DX10 (BC5, 7)

    // DXGI Enum values
    enum class DXGI_FORMAT : uint32_t
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
        uint32_t size;
        uint32_t flags;
        uint32_t fourCC;
        uint32_t rgbBitCount;
        uint32_t rBitMask;
        uint32_t gBitMask;
        uint32_t bBitMask;
        uint32_t aBitMask;
    };

    struct DDS_HEADER {
        uint32_t size;
        uint32_t flags;
        uint32_t height;
        uint32_t width;
        uint32_t pitchOrLinearSize;
        uint32_t depth;
        uint32_t mipmapCount;
        uint32_t reserved1[11];
        DDS_PIXELFORMAT ddspf;
        uint32_t caps;
        uint32_t caps2;
        uint32_t caps3;
        uint32_t caps4;
        uint32_t reserved2;
    };

    struct DDS_HEADER_DXT10 {
        uint32_t dxgiFormat;
        uint32_t resourceDimension;
        uint32_t miscFlag;
        uint32_t arraySize;
        uint32_t miscFlags2;
    };
    #pragma pack(pop)
}

