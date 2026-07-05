/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2026 isaki */

#pragma once

#include <cstdint>
#include <cstddef>
#include <filesystem>
#include <stdexcept>

namespace isaki::xivte
{
    inline constexpr uint32_t BPP_MASK = 0xF0;
    inline constexpr size_t BPP_SHIFT = 0x4;
    inline constexpr uint32_t FTYPE_MASK = 0XF000;
    inline constexpr size_t FTYPE_SHIFT = 0xC;

    inline constexpr uint32_t FTYPE_BC123 = 0x3;
    inline constexpr uint32_t FTYPE_BC57 = 0x6;

    enum class XIVTexAttribute : uint32_t
    {
        Texture2D = 0x00800000
    };

    enum class XIVTexFormat : uint32_t {
        UNKNOWN = 0xFFFFFFFF,   // -1

        // Integer Types
        B8G8R8A8 = 0x1450,

        // Compression Types
        BC1 = 0x3420,
        BC3 = 0x3431,
        BC5 = 0x6230,
        BC7 = 0x6432
    };

    #pragma pack(push, 1)
    struct XIVMipConfig
    {
    public:
        XIVMipConfig() = default;
        explicit XIVMipConfig(const uint8_t raw) : m_raw(raw) {};
        operator uint8_t() const { return m_raw; }

        void count(const uint8_t value)
        {
            if (value > 0x7F) {
                throw std::runtime_error("Invalid mip count: exceeds 7 bits (max 127)");
            }

            // Bitwise operations are host-endian agnostic; they always target logical bits
            m_raw = (m_raw & 0x80) | (value & 0x7F);
        }

        uint8_t count() const {
            return m_raw & 0x7F;
        }

        // Layout: Bit 7 represents the flag (0x80)
        void unknownFlag(bool value) {
            m_raw = value ? (m_raw | 0x80) : (m_raw & 0x7F);
        }

        bool unknownFlag() const {
            return (m_raw & 0x80) != 0;
        }

    private:
        uint8_t m_raw;
    };

    struct XIVTexHeader
    {
        uint32_t type;              // XIVTexAttribute
        uint32_t format;            // XIVTexFormat
        uint16_t width;
        uint16_t height;
        uint16_t depth;
        XIVMipConfig mipLevels;
        uint8_t arraySize;
        uint32_t lodOffset[3];

        uint32_t offsetToSurface[13];
    };
    #pragma pack(pop)
    static_assert(sizeof(XIVMipConfig) == 1);
    static_assert(sizeof(XIVTexHeader) == 80);

    class Texture
    {
    public:
        Texture() = delete;
        Texture(const Texture&) = delete;
        Texture& operator=(const Texture&) = delete;

        ~Texture();

        explicit Texture(const std::filesystem::path& ddsFile);
        Texture(Texture&& o) noexcept;
        Texture& operator=(Texture&& o) noexcept;

        void save(const std::filesystem::path& texFile) const;

    private:
        // Stack Allocated
        size_t m_dataLength;
        XIVTexHeader m_xivHeader;

        // "Dangerous" memory access
        char* m_dds;
    };
}
