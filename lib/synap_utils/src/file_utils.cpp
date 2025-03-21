// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.

#include "synap/file_utils.hpp"
#include "synap/logging.hpp"

#include <fstream>
#include <iostream>

#ifdef ENABLE_STD_FILESYSTEM
#include <filesystem>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdlib>
#endif

using namespace std;

namespace synaptics {
namespace synap {

bool file_exists(const string& file_name)
{
    ifstream f(file_name.c_str());
    return f.good() && !directory_exists(file_name);
}

std::string file_find_up(const string& file_name, const string& path, int levels)
{
    string file_path = file_name;
    while (levels-- >= 0) {
        string file_full_path = path + file_path;
        if (file_exists(file_full_path)) {
            LOGV << "File found: " << file_full_path;
            return file_full_path;
        }
        file_path = "../" + file_path;
    }
    LOGI << "File not found: " << file_name;
    return {};
}

string filename_extension(const string& file_name)
{
    size_t i = file_name.rfind('.', file_name.length());
    if (i != string::npos) {
        return (file_name.substr(i + 1, file_name.length() - i));
    }

    return "";
}

string filename_without_extension(const string& file_name)
{
    size_t i = file_name.rfind('/', file_name.length());
    string filename_nopath;
    if (i != string::npos) {
        // Remove path
        filename_nopath = file_name.substr(i + 1, file_name.length() - i);
    }
    else {
        filename_nopath = file_name;
    }
    i = filename_nopath.rfind('.', filename_nopath.length());
    if (i != string::npos) {
        // Remove extension
        return (filename_nopath.substr(0, i));
    }

    return file_name;
}

std::string filename_path(const std::string& file_name)
{
    size_t i = file_name.rfind('/', file_name.length());
    if (i == string::npos) {
        return "";
    }
    return file_name.substr(0, i + 1);
}


string file_read(const string& file_name)
{
    ifstream f(file_name);
    if (!f.good()) {
        LOGE << "Error: can't read input file: " << file_name;
        return {};
    }
    string file_content(istreambuf_iterator<char>{f}, {});
    return file_content;
}


vector<uint8_t> binary_file_read(const string& file_name)
{
    ifstream f(file_name, ios::binary);
    if (!f.good()) {
        LOGE << "Error: can't read input file: " << file_name;
        return {};
    }
    vector<uint8_t> file_content(istreambuf_iterator<char>{f}, {});
    return file_content;
}


bool binary_file_write(const string& file_name, const void* data, size_t size)
{
    ofstream wf;
    wf.open(file_name, ios::out | ios::binary);
    if (!wf.good()) {
        LOGE << "Error: can't open file from writing: " << file_name;
        return false;
    }

    if (data) {
        wf.write(static_cast<const char*>(data), size);
    }
    wf.close();
    return wf.good();
}

#ifdef ENABLE_STD_FILESYSTEM
bool create_directory(const string& out_dir)
{
    return std::filesystem::create_directory(out_dir);
}

bool create_directories(const string& out_dir)
{
    return std::filesystem::create_directories(out_dir);
}

bool directory_exists(const std::string& directory_name)
{
    return filesystem::is_directory(directory_name);
}


#else

bool create_directory(const string& out_dir)
{
    struct stat st;
    if (stat(out_dir.c_str(), &st) == -1) {
        if (!mkdir(out_dir.c_str(), 0755)) {
            return true;
        }
    }
    LOGE << "cannot create directory " << out_dir;
    return false;
}

bool create_directories(const string& out_dir)
{
    const std::string cmd = "mkdir -p " + out_dir;
    int ret = system(cmd.c_str());
    if (ret != 0) {
        LOGE << "cannot create directory " << out_dir;
        return false;
    }
    return true;
}

bool directory_exists(const std::string& directory_name)
{
    struct stat sb;
    return stat(directory_name.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode);
}

#endif

}  // namespace synap
}  // namespace synaptics
