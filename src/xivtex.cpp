/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2026 isaki */

#include <filesystem>
#include <cstdint>
#include <cstddef>
#include <cstring>      // memcmp
#include <stdexcept>
#include <utility>
#include <type_traits>

#include <fstream>
#include <endian.h>

#include <algorithm>
#include <optional>

#include "xivte/dds.hpp"
#include "xivte/xivtex.hpp"

namespace fs = std::filesystem;
namespace xte = isaki::xivte;

namespace
{
    // Little Endian, so first in list, is last in numbers.
    // NOTE: This might not be represented this way in memory
    // on a big-endian system, so conversions are required.
    constexpr uint32_t CHANNEL_0 = 0x000000FF;
    constexpr uint32_t CHANNEL_2 = 0x00FF0000;

    constexpr uint32_t write_xiv_header(const uint32_t headerValue) noexcept
    {
        return htole32(headerValue);
    }

    constexpr uint16_t write_xiv_header(const uint16_t headerValue) noexcept
    {
        return htole16(headerValue);
    }

    template<typename T> requires std::is_enum_v<T>
    constexpr uint32_t write_xiv_header_enum(const T headerValue) noexcept
    {
        const uint32_t host = static_cast<uint32_t>(headerValue);
        return write_xiv_header(host);
    }

    constexpr uint32_t read_dds_header(const uint32_t headerValue) noexcept
    {
        return le32toh(headerValue);
    }

    template<typename T> requires std::is_enum_v<T>
    constexpr T read_dds_header_enum(const uint32_t headerValue) noexcept
    {
        const uint32_t host = read_dds_header(headerValue);
        return static_cast<T>(host);
    }

    xte::XIVTexFormat get_texture_format_dx10(const xte::DDS_HEADER_DXT10& x) noexcept
    {
        const xte::DXGI_FORMAT df = read_dds_header_enum<xte::DXGI_FORMAT>(x.dxgiFormat);

        xte::XIVTexFormat ret;

        switch(df)
        {
            case xte::DXGI_FORMAT::B8G8R8A8_UNORM:
            case xte::DXGI_FORMAT::R8G8B8A8_UNORM:
                ret = xte::XIVTexFormat::B8G8R8A8;
                break;

            case xte::DXGI_FORMAT::BC1_UNORM:
                ret = xte::XIVTexFormat::BC1;
                break;

            case xte::DXGI_FORMAT::BC3_UNORM:
                ret = xte::XIVTexFormat::BC3;
                break;

            case xte::DXGI_FORMAT::BC5_UNORM:
                ret = xte::XIVTexFormat::BC5;
                break;

            case xte::DXGI_FORMAT::BC7_UNORM:
                ret = xte::XIVTexFormat::BC7;
                break;

            default:
                ret = xte::XIVTexFormat::UNKNOWN;
                break;
        }

        return ret;
    }

    xte::XIVTexFormat get_texture_format_zero(const xte::DDS_PIXELFORMAT& pf) noexcept
    {
        if (read_dds_header(pf.rgbBitCount) == 32)
        {
            const uint32_t rmask = read_dds_header(pf.rBitMask);
            const uint32_t bmask = read_dds_header(pf.bBitMask);

            if (rmask == CHANNEL_2 && bmask == CHANNEL_0)
            {
                // BGRA
                return xte::XIVTexFormat::B8G8R8A8;
            }
        }

        return xte::XIVTexFormat::UNKNOWN;
    }

    xte::XIVTexFormat get_texture_format(const xte::DDS_HEADER& h, const std::optional<xte::DDS_HEADER_DXT10>& x) noexcept
    {
        // H will not be null, but x might.
        const uint32_t fourCC = read_dds_header(h.ddspf.fourCC);

        xte::XIVTexFormat ret;
        switch(fourCC)
        {
            case xte::DDS_DX10:
                ret = (x)
                    ? get_texture_format_dx10(x.value())
                    : xte::XIVTexFormat::UNKNOWN;

                break;

            case xte::DDS_DXT1:
                ret = xte::XIVTexFormat::BC1;
                break;

            case xte::DDS_DXT5:
                ret = xte::XIVTexFormat::BC3;
                break;

            case 0:
                // This can absolutely happen on some DDS files.
                ret = get_texture_format_zero(h.ddspf);
                break;

            default:
                // Something is wrong
                ret = xte::XIVTexFormat::UNKNOWN;
                break;
        }

        return ret;
    }

    // This operates on host endianness!
    uint32_t getMipSize(size_t mmidx, uint32_t w, uint32_t h, xte::XIVTexFormat format)
    {
        const uint32_t bpp = 1 << ((static_cast<uint32_t>(format) & xte::BPP_MASK) >> xte::BPP_SHIFT);
        const uint32_t formatType = (static_cast<uint32_t>(format) & xte::FTYPE_MASK) >> xte::FTYPE_SHIFT;

        w = std::max(1u, w >> mmidx);
        h = std::max(1u, h >> mmidx);

        uint32_t ret;
        if (formatType == xte::FTYPE_BC57 || formatType == xte::FTYPE_BC123)
        {
            const uint32_t nbw = std::max(1u, (w + 3) >> 2);
            const uint32_t nbh = std::max(1u, (h + 3) >> 2);
            ret = (nbw * nbh * bpp) << 1;
        }
        else
        {
            switch (format)
            {
                case xte::XIVTexFormat::B8G8R8A8:
                    ret = (h * w * bpp) >> 3;
                    break;

                default:
                    throw std::invalid_argument("Specified format is not supported");
            };
        }

        return ret;
    }

    void populate_xiv_header(xte::XIVTexHeader& header, const xte::DDS_HEADER& dds, const std::optional<xte::DDS_HEADER_DXT10>& dx10)
    {
        const xte::XIVTexFormat format = get_texture_format(dds, dx10);
        if (format == xte::XIVTexFormat::UNKNOWN)
        {
            throw std::runtime_error("Failed to deteremine XIVTexFormat");
        }


        // Calculations First.

        const uint32_t widthHost = read_dds_header(dds.width);
        const uint32_t heightHost = read_dds_header(dds.height);
        const uint32_t mipCountHost = std::max(1u, read_dds_header(dds.mipmapCount));
        const uint32_t depthHost = std::max(1u, read_dds_header(dds.depth));
        const uint32_t arraySizeHost = (dx10) ? read_dds_header(dx10.value().arraySize) : 0u;

        memset(&header, 0, sizeof(xte::XIVTexHeader));

        header.type = write_xiv_header_enum(xte::XIVTexAttribute::Texture2D);
        header.format = write_xiv_header_enum(format);
        header.width = write_xiv_header(static_cast<uint16_t>(widthHost));
        header.height = write_xiv_header(static_cast<uint16_t>(heightHost));
        header.depth = write_xiv_header(static_cast<uint16_t>(depthHost));

        // When we cast these down, we get 1 byte, which is endian agnostic.
        header.arraySize = static_cast<uint8_t>(arraySizeHost);
        header.mipLevels.count(static_cast<uint8_t>(mipCountHost));

        uint32_t currentOffset = sizeof(xte::XIVTexHeader);

        // Since we memset to 0, we know the remaining entries, if any, are zero.
        const size_t loopCutoff = std::min(sizeof(xte::XIVTexHeader::offsetToSurface) / sizeof(uint32_t), static_cast<size_t>(mipCountHost));

        for (size_t i = 0; i < loopCutoff; ++i)
        {
            header.offsetToSurface[i] = write_xiv_header(currentOffset);

            const uint32_t levelSize = getMipSize(i, widthHost, heightHost, format);
            currentOffset += levelSize;
        }

        // Calculate LoD Offsets
        uint32_t lod1, lod2;

        switch (mipCountHost)
        {
            case 1:
                lod1 = 0;
                lod2 = 0;
                break;

            case 2:
                lod1 = 1;
                lod2 = 1;
                break;

            default:
                lod1 = 1;
                lod2 = 2;
                break;
        };

        header.lodOffset[0] = 0;
        header.lodOffset[1] = write_xiv_header(lod1);
        header.lodOffset[2] = write_xiv_header(lod2);
    }
}

//
// Object Lifecycle
//

xte::Texture::~Texture()
{
    // Delete is nullsafe
    delete[] m_dds;
}

xte::Texture::Texture(const fs::path& ddsFile) :
    m_dataLength(0),
    m_dds(nullptr)
{
    // Do not assign internal state until things are clean
    char* tmp = nullptr;
    size_t dataLength = 0;

    try
    {
        std::ifstream is;
        is.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        is.open(ddsFile, std::ios::binary | std::ios::in);

        // Read the first four bytes.
        char data[sizeof(xte::DDS_MAGIC)];
        is.read(data, sizeof(data));

        if (std::memcmp(data, xte::DDS_MAGIC, sizeof(data)) != 0)
        {
            throw std::runtime_error("Input file is not a DDS file");
        }

        // Read the header.
        DDS_HEADER head;
        is.read(reinterpret_cast<char*>(&head), sizeof(DDS_HEADER));

        std::optional<DDS_HEADER_DXT10> dx10 = std::nullopt;
        if (read_dds_header(head.ddspf.fourCC) == DDS_DX10)
        {
            dx10.emplace();
            is.read(reinterpret_cast<char*>(&dx10), sizeof(DDS_HEADER_DXT10));
        }

        // We need the rest.
        const std::ifstream::pos_type dataStart = is.tellg();
        if (dataStart == static_cast<std::ifstream::pos_type>(-1))
        {
            throw std::runtime_error("Failed to determine texture data start");
        }

        is.seekg(0, std::ios::end);
        const std::ifstream::pos_type dataEnd = is.tellg();
        if (dataEnd == static_cast<std::ifstream::pos_type>(-1))
        {
            throw std::runtime_error("Failed to determine texture data end");
        }

        is.clear();
        is.seekg(dataStart);

        // Size of data, dataStart is 0, and dataEnd is 1 past the last byte.
        // Thus, no need to add 1.
        dataLength = static_cast<size_t>(dataEnd - dataStart);
        tmp = new char[dataLength];

        is.read(tmp, dataLength);

        is.close();

        populate_xiv_header(m_xivHeader, head, dx10);
    }
    catch (...)
    {
        delete[] tmp;
        throw;
    }

    m_dataLength = dataLength;
    m_dds = tmp;
}

xte::Texture::Texture(Texture&& o) noexcept :
    m_dataLength(0),
    m_xivHeader(std::move(o.m_xivHeader)),
    m_dds(o.m_dds)
{
    o.m_dds = nullptr;
}

xte::Texture& xte::Texture::operator=(Texture&& o) noexcept
{
    if (&o != this)
    {
        delete[] m_dds;
        m_dataLength = o.m_dataLength;
        m_xivHeader = std::move(o.m_xivHeader);
        m_dds = o.m_dds;

        o.m_dds = nullptr;
    }

    return *this;
}

void isaki::xivte::Texture::save(const fs::path& texFile) const
{
    std::ofstream os;
    os.exceptions(std::ofstream::failbit | std::ofstream::badbit);

    os.open(texFile, std::ios::binary | std::ios::out);

    os.write(reinterpret_cast<const char*>(&m_xivHeader), sizeof(XIVTexHeader));
    os.write(m_dds, m_dataLength);

    os.close();
}
