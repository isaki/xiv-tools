/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2026 isaki */

#pragma once

#include <cstddef>
#include <string_view>

#include "xiv/iofwd.hpp"

namespace isaki::xiv
{
    class PString
    {
    public:
        PString() = delete;

        ~PString();

        PString(std::size_t size, char padding);

        PString(const PString& other);
        PString& operator=(const PString& other);

        PString(PString&& other) noexcept;
        PString& operator=(PString&& other) noexcept;

        void write(XIVDataWriter& writer);
        void read(XIVDataReader& reader);

        void set(std::string_view view);

        // Cast handling
        explicit operator std::string_view() const;

    private:
        char* m_string;
        std::size_t m_strlen;
        std::size_t m_size;
        char m_padding;
    };
}
