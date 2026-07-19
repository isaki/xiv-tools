/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2025-2026 isaki */

#include <iostream>
#include <string_view>

#include "xiv_internal/config.hpp"
#include "xiv/version.hpp"

namespace xiv = isaki::xiv;

void xiv::print_version(std::ostream& os, std::string_view name, int boostVersion)
{
    const int bvPatch = boostVersion % 100;
    const int bvMinor = boostVersion / 100 % 100;
    const int bvMajor = boostVersion / 100000;

    os << name
        << " ("
        << cmake::project_name
        << ") v"
        << cmake::project_version
        << std::endl;

    os << "Copyright (C) 2026 isaki@github" << std::endl;
    os << "License: GNU GPL version 2 or later <https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html>" << std::endl;
    os << name << " comes with NO WARRANTY, to the extent permitted by law" << std::endl;

    os << "Home page: <" << cmake::project_url << '>' << std::endl;

    os << "Compiled with: "
        << cmake::cxx_compiler
        << " v"
        << cmake::cxx_compiler_ver
        << " for "
        << cmake::build_platform
        << ' '
        << cmake::build_architecture
        << std::endl;

    os << "Boost: "
        << bvMajor
        << '.'
        << bvMinor
        << '.'
        << bvPatch
        << std::endl;
}
