/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2026 isaki */

#pragma once

#include <ios>
#include <filesystem>
#include <type_traits>

#include "xiv/endian.hpp"

namespace isaki::xiv
{
    class XIVDataReader
    {
    public:
        XIVDataReader() = delete;
        XIVDataReader(const XIVDataReader&) = delete;
        XIVDataReader& operator=(const XIVDataReader&) = delete;
        XIVDataReader(XIVDataReader&&) = delete;
        XIVDataReader& operator=(XIVDataReader&&) = delete;

        ~XIVDataReader();
        explicit XIVDataReader(const std::filesystem::path& file);

        void read(char* buffer, std::streamsize length);

        void close();

        void seekg(std::streampos pos);

        void seekg(std::streamoff, std::ios::seekdir direction);

        std::streampos tellg();

        template <typename T>
        requires std::is_integral_v<T> || std::is_floating_point_v<T>
        T read()
        {
            T value;
            read(reinterpret_cast<char*>(&value), sizeof(value));

            if constexpr (std::is_same_v<T, float>)
            {
                return le32ftoh(value);
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                return le64ftoh(value);
            }
            else if constexpr (sizeof(T) == sizeof(uint16_t))
            {
                return le16toh(value);
            }
            else if constexpr (sizeof(T) == sizeof(uint32_t))
            {
                return le32toh(value);
            }
            else if constexpr (sizeof(T) == sizeof(uint64_t))
            {
                return le64toh(value);
            }
            else
            {
                return value;
            }
        }

        template <typename E>
        requires std::is_enum_v<E>
        E readEnum()
        {
            return static_cast<E>(read<std::underlying_type_t<E>>());
        }

    private:
        std::istream* m_is;
    };

    class XIVDataWriter
    {
    public:
        XIVDataWriter() = delete;
        XIVDataWriter(const XIVDataWriter&) = delete;
        XIVDataWriter& operator=(const XIVDataWriter&) = delete;
        XIVDataWriter(XIVDataWriter&&) = delete;
        XIVDataWriter& operator=(XIVDataWriter&&) = delete;

        ~XIVDataWriter();
        explicit XIVDataWriter(const std::filesystem::path& file);

        void write(const char* buffer, std::streamsize length);

        void close();

        void seekp(std::streampos pos);

        void seekp(std::streamoff, std::ios::seekdir direction);

        std::streampos tellp();

        template <typename T>
        requires std::is_integral_v<T> || std::is_floating_point_v<T>
        void write(T value)
        {
            T buffer;
            if constexpr (std::is_same_v<T, float>)
            {
                buffer = htole32f(value);
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                buffer = htole64f(value);
            }
            else if constexpr (sizeof(T) == sizeof(uint16_t))
            {
                buffer = htole16(value);
            }
            else if constexpr (sizeof(T) == sizeof(uint32_t))
            {
                buffer = htole32(value);
            }
            else if constexpr (sizeof(T) == sizeof(uint64_t))
            {
                buffer = htole64(value);
            }
            else
            {
                buffer = value;
            }

            write(reinterpret_cast<char*>(&buffer), sizeof(T));
        }

        template <typename E>
        requires std::is_enum_v<E>
        void writeEnum(E value)
        {
            write(static_cast<std::underlying_type_t<E>>(value));
        }

    private:
        std::ostream* m_os;
    };
}
