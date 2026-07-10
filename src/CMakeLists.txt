# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright 2025-2026 isaki

add_executable(xiv-tex-encode
    texture.cpp
    version.cpp
    main.cpp
)

target_include_directories(xiv-tex-encode
    PRIVATE
    "${PROJECT_BINARY_DIR}/configured_files/include"
    "${PROJECT_SOURCE_DIR}/include"
)

target_link_libraries(xiv-tex-encode PRIVATE Boost::program_options)
