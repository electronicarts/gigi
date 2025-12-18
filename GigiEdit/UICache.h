///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace UICache
{
    struct VariableNameList
    {
        std::vector<std::string> names;
        float maxTextWidth = 0.0f;
    };

    struct State
    {
        std::unordered_map<size_t, VariableNameList> variableNameListCache;
    };

    extern State s_state;

    inline void OnFrame()
    {
        s_state.variableNameListCache.clear();
    }

    template <typename LAMBDA>
    const VariableNameList& GetListOfVariableNames(size_t hash, LAMBDA& lambda)
    {
        // Use an exisiting list if we have one
        {
            auto it = s_state.variableNameListCache.find(hash);
            if (it != s_state.variableNameListCache.end())
                return it->second;
        }

        // else make one
        s_state.variableNameListCache[hash] = std::move(lambda());
        return s_state.variableNameListCache[hash];
    }
};
