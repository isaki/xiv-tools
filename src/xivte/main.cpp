/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright 2025-2026 isaki */

#include <iostream>
#include <filesystem>
#include <string>
#include <string_view>
#include <optional>
#include <cctype>
#include <algorithm>

#include <boost/version.hpp>
#include <boost/program_options.hpp>

#include "xiv/version.hpp"
#include "xivte/texture.hpp"

namespace po = boost::program_options;
namespace xte = isaki::xivte;
namespace x = isaki::xiv;
namespace fs = std::filesystem;

namespace
{
    constexpr std::string_view DDS_EXT = ".dds";
    constexpr std::string_view TEX_EXT = ".tex";

    fs::path build_tex_file_path(const fs::path& ddsFile, const std::optional<fs::path>& outputDir = std::nullopt)
    {
        fs::path ret = (outputDir) ? outputDir.value() / ddsFile.filename() : ddsFile;

        std::string ext = ret.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });

        if (ext == DDS_EXT)
        {
            ret.replace_extension(TEX_EXT);
        }
        else
        {
            ret.concat(TEX_EXT);
        }

        return ret;
    }

    std::string argv_basename(const char * name)
    {
        const std::string_view tmp(name);
        const fs::path p(tmp);
        return p.filename().string();
    }

    void print_help(std::ostream& os, const std::string_view name, const po::options_description& desc)
    {
        os << name << " <file.dds>\n" << std::endl;
        os << desc << std::endl;
    }
}

int main(int argc, char ** argv)
{
    try
    {
        // Declare the supported options.
        po::options_description desc("Options");
        desc.add_options()
            ("help,h", "Print this message.")
            ("version,v", "Display version information.")
            ("output-dir", po::value<std::string>(), "Optional directory to output the TEX file to.")
        ;

        po::options_description hidden("Hidden options");
        hidden.add_options()
            ("dds", po::value<std::string>(), "The DDS file to convert")
        ;

        po::options_description all;
        all.add(desc).add(hidden);

        po::positional_options_description posdesc;
        posdesc.add("dds", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(all).positional(posdesc).run(), vm);
        po::notify(vm);

        if (vm.contains("version"))
        {
            const std::string name = argv_basename(argv[0]);
            x::print_version(std::cout, name, BOOST_VERSION);
            return 0;
        }

        if (vm.contains("help")) {
            const std::string name = argv_basename(argv[0]);
            print_help(std::cout, name, desc);
            return 0;
        }

        if (!vm.contains("dds"))
        {
            std::cerr << "Invalid usage; please run with --help" << std::endl;
            return 1;
        }

        fs::path ddsFile(vm["dds"].as<std::string>());
        if (!fs::exists(ddsFile))
        {
            std::cerr << "Unable to locate " << ddsFile << std::endl;
            return 2;
        }

        ddsFile = fs::canonical(ddsFile);

        fs::path texFile;
        if (vm.contains("output-dir"))
        {
            fs::path outputDir(vm["output-dir"].as<std::string>());
            if (!fs::is_directory(outputDir))
            {
                std::cerr << "--output-dir is not a valid directory" << std::endl;
                return 3;
            }

            outputDir = fs::canonical(outputDir);

            const std::optional o(outputDir);
            texFile = build_tex_file_path(ddsFile, o);
        }
        else
        {
            texFile = build_tex_file_path(ddsFile);
        }

        // Do the teture things.
        std::cout << "DDS File: " << ddsFile << std::endl
            << "TEX File: " << texFile << std::endl;

        std::cout << "Loading DDS data..." << std::endl;

        xte::Texture tex(ddsFile);

        std::cout << "Writing TEX data..." << std::endl;

        tex.save(texFile);

        std::cout << "Operation COMPLETE!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return 10;
    }

    return 0;
}
