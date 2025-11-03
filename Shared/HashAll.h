///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2025 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

static inline size_t hash_combine(size_t A, size_t B)
{
    return A ^ (B + 0x9e3779b9 + (A << 6) + (A >> 2));
}

template<typename T, typename... Args>
static inline size_t HashAll(T&& first, Args&&... args)
{
    size_t firstHash = std::hash<std::decay_t<T>>{}(std::forward<T>(first));
    if constexpr (sizeof...(args) == 0)
        return firstHash;
    else
        return hash_combine(firstHash, HashAll(std::forward<Args>(args)...));
}
