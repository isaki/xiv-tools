/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2026 isaki */

#pragma once

#include <filesystem>
#include <cstddef>
#include "xivte/xivtex.hpp"

namespace isaki::xivte
{
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
        std::size_t m_dataLength;
        XIVTexHeader m_xivHeader;

        // "Dangerous" memory access
        char* m_dds;
    };
}
