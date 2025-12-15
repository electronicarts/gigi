///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <string>

inline void CaseInsensitiveSort(std::vector<std::string>& strs)
{
    std::sort(
        std::begin(strs),
        std::end(strs),
        [](const std::string& str1, const std::string& str2) {
            return lexicographical_compare(
                begin(str1), end(str1),
                begin(str2), end(str2),
                [](const char& char1, const char& char2) {
                    return tolower(char1) < tolower(char2);
                }
            );
        }
    );
}
