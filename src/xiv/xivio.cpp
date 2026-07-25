/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2025-2026 isaki */

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "xiv/xivio.hpp"

namespace xiv = isaki::xiv;
namespace fs = std::filesystem;

//
// READER
//

xiv::XIVDataReader::~XIVDataReader()
{
    // This is nullsafe.
    delete m_is;
}

xiv::XIVDataReader::XIVDataReader(const fs::path& file) : m_is(nullptr)
{
    std::ifstream* tmp = nullptr;
    try
    {
        tmp = new std::ifstream();
        tmp->exceptions(std::ifstream::failbit | std::ifstream::badbit);
        tmp->open(file, std::ifstream::binary | std::ifstream::in);
    }
    catch (...)
    {
        delete tmp;
        throw;
    }

    m_is = tmp;
}

void xiv::XIVDataReader::read(char* buffer, size_t length)
{
    if (m_is == nullptr) [[unlikely]]
    {
        throw std::runtime_error("Attempt to read a closed stream");
    }

    m_is->read(buffer, static_cast<std::streamsize>(length));
}

void xiv::XIVDataReader::close()
{
    // Close must be idempotent. So, we just ignore null.
    std::ifstream* tmp = dynamic_cast<std::ifstream*>(m_is);
    if (tmp == nullptr)
    {
        // Either it's closed or it can't be closed.
        return;
    }

    // Tmp owns this now, no longer our problem.
    m_is = nullptr;

    try
    {
        // Allow this to throw, if we delete, it won't.
        tmp->close();
        delete tmp;
    }
    catch (...)
    {
        // Delete anyway
        delete tmp;
        throw;
    }
}

//
// WRITER
//

xiv::XIVDataWriter::~XIVDataWriter()
{
    // This is nullsafe.
    delete m_os;
}

xiv::XIVDataWriter::XIVDataWriter(const fs::path& file) : m_os(nullptr)
{
    std::ofstream* tmp = nullptr;
    try
    {
        tmp = new std::ofstream();
        tmp->exceptions(std::ofstream::failbit | std::ofstream::badbit);
        tmp->open(file, std::ofstream::binary | std::ofstream::out);
    }
    catch (...)
    {
        delete tmp;
        throw;
    }

    m_os = tmp;
}

void xiv::XIVDataWriter::write(const char* buffer, size_t length)
{
    if (m_os == nullptr) [[unlikely]]
    {
        throw std::runtime_error("Attempt to write to a closed stream");
    }

    m_os->write(buffer, static_cast<std::streamsize>(length));
}

void xiv::XIVDataWriter::close()
{
    // Close must be idempotent. So, we just ignore null.
    std::ofstream* tmp = dynamic_cast<std::ofstream*>(m_os);
    if (tmp == nullptr)
    {
        // Either it's closed or it can't be closed.
        return;
    }

    // Tmp owns this now, no longer our problem.
    m_os = nullptr;

    try
    {
        // Allow this to throw, if we delete, it won't.
        tmp->close();
        delete tmp;
    }
    catch (...)
    {
        // Delete anyway
        delete tmp;
        throw;
    }
}
