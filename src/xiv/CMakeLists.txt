# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2025-2026 isaki

add_library(xivCore STATIC
    version.cpp
)

target_include_directories(xivCore
    PRIVATE
    "${PROJECT_BINARY_DIR}/configured_files/include"
    "${PROJECT_SOURCE_DIR}/include"
)
