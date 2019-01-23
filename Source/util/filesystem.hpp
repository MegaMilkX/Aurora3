#ifndef UTIL_FILESYSTEM_H
#define UTIL_FILESYSTEM_H

#include "win32_module.hpp"
#include <shlwapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <iterator>

inline std::string cut_dirpath(const std::string& path)
{
    return std::string(
        path.begin(), 
        path.begin() + path.find_last_of("\\")
    );
}

inline std::string get_module_dir()
{
    std::string filename;
    char buf[512];
    GetModuleFileNameA(this_module_handle(), buf, 512);
    filename = buf;
    filename = cut_dirpath(filename);
    return filename;
}

inline bool copy_file(const std::string& from, const std::string& to)
{
    if(CopyFileA(
        from.c_str(),
        to.c_str(),
        false
    ) == FALSE)
    {
        return false;
    }
    return true;
}

inline void find_files(const std::string& dir, const std::string& filter, std::vector<std::string>& out) {
    std::string full_filter = dir + "\\*";
    WIN32_FIND_DATAA data;
    HANDLE hFind = FindFirstFileA(full_filter.c_str(), &data);

    char buf[260];
    DWORD len = GetFullPathNameA(full_filter.c_str(), 260, buf, 0);
    std::string dirpath(buf, len);

    if ( hFind != INVALID_HANDLE_VALUE ) 
    {
        do 
        {
            if(std::string(data.cFileName) == "." || 
                std::string(data.cFileName) == "..")
            {
                continue;
            }

            if(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                find_files(dir + "\\" + std::string(data.cFileName), filter, out);
                continue;
            }

            if(PathMatchSpecA(data.cFileName, filter.c_str())) {
                out.push_back(dir + "\\" + std::string(data.cFileName));
            }
        } while (FindNextFileA(hFind, &data));
        FindClose(hFind);
    }
}

inline std::vector<std::string> find_all_files(const std::string& dir, const std::string& filter)
{
    std::vector<std::string> names;
    find_files(dir, filter, names);
    return names;
}

#endif
