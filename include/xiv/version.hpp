/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2026 isaki */

#pragma once

#include <iosfwd>
#include <string_view>

namespace isaki::xiv
{
    void print_version(std::ostream& os, std::string_view name, int boostVersion);
}
