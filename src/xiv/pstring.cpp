/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2025-2026 isaki */

#include <cstddef>
#include <cstring>
#include <string_view>
#include <ios>
#include <stdexcept>

#include "xiv/io.hpp"
#include "xiv/pstring.hpp"

namespace xiv = isaki::xiv;

namespace
{
    constexpr std::size_t pstrlen(const char* s, std::size_t max)
    {
        const std::string_view view(s, max);
        const std::string_view::size_type vlen = view.find('\0');
        return (vlen == std::string_view::npos) ? max : static_cast<std::size_t>(vlen);
    }
}

xiv::PString::~PString()
{
    delete[] m_string;
}

xiv::PString::PString(std::size_t size, char padding) :
    m_string(nullptr),
    m_strlen(0),
    m_size(size),
    m_padding(padding)
{
    // Memset to zero
    m_string = new char[m_size]();
}

xiv::PString::PString(const PString& other) :
    m_string(nullptr),
    m_strlen(other.m_strlen),
    m_size(other.m_size),
    m_padding(other.m_padding)
{
    // We don't need memset as we are doing a full overwrite.
    m_string = new char[m_size];
    std::memcpy(m_string, other.m_string, m_size);
}

xiv::PString& xiv::PString::operator=(const PString& other)
{
    if (this != &other)
    {
        // Temp buffer, we throw before things go bad.
        char* tmp = new char[other.m_size];

        // No more throw risk, proceed safely.
        std::memcpy(tmp, other.m_string, other.m_size);

        delete[] m_string;
        m_string = tmp; // Take ownership of the temp buffer.
        m_strlen = other.m_strlen;
        m_size = other.m_size;
        m_padding = other.m_padding;
    }

    return *this;
}

xiv::PString::PString(PString&& other) noexcept :
    m_string(other.m_string),
    m_strlen(other.m_strlen),
    m_size(other.m_size),
    m_padding(other.m_padding)
{
    other.m_string = nullptr;
    other.m_strlen = 0;
    other.m_size = 0;
}

xiv::PString& xiv::PString::operator=(PString&& other) noexcept
{
    if (this != &other)
    {
        delete[] m_string;
        m_string = other.m_string;
        m_strlen = other.m_strlen;
        m_size = other.m_size;
        m_padding = other.m_padding;

        other.m_string = nullptr;
        other.m_strlen = 0;
        other.m_size = 0;
    }

    return *this;
}

void xiv::PString::read(XIVDataReader& reader)
{
    // Read raw.
    reader.read(m_string, static_cast<std::streamsize>(m_size));
    m_strlen = pstrlen(m_string, m_size);
}

void xiv::PString::write(XIVDataWriter& writer)
{
    writer.write(m_string, static_cast<std::streamsize>(m_strlen));

    if (m_strlen < m_size)
    {
        writer.write<char>('\0');

        const std::size_t remaining = m_size - m_strlen - 1;
        for (std::size_t i = 0; i < remaining; ++i)
        {
            writer.write<char>(m_padding);
        }
    }
}

void xiv::PString::set(std::string_view view)
{
    const std::size_t vlen = static_cast<std::size_t>(view.length());
    if (vlen > m_size)
    {
        throw std::out_of_range("Attempt to set string larger than padded string buffer");
    }

    std::memcpy(m_string, view.data(), vlen);

    if (vlen < m_size)
    {
        m_string[vlen] = '\0';
    }

    m_strlen = vlen;
}

xiv::PString::operator std::string_view() const
{
    std::string_view ret(m_string, m_strlen);
    return ret;
}
