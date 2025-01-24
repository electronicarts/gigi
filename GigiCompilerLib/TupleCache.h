///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <unordered_map>
#include <tuple>

// Portions of this software were based on https://stackoverflow.com/questions/20834838/using-tuple-in-unordered-map
// function has to live in the std namespace 
// so that it is picked up by argument-dependent name lookup (ADL).
namespace std {
    namespace
    {

        // Portions of this software were based on code from boost
        // Reciprocal of the golden ratio helps spread entropy
        //     and handles duplicates.
        // See Mike Seymour in magic-numbers-in-boosthash-combine:
        //     https://stackoverflow.com/questions/4948780

        template <class T>
        inline void hash_combine(std::size_t& seed, T const& v)
        {
            seed ^= hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
        }

        // Recursive template code derived from Matthieu M.
        template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
        struct HashValueImpl
        {
            static void apply(size_t& seed, Tuple const& tuple)
            {
                HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
                hash_combine(seed, get<Index>(tuple));
            }
        };

        template <class Tuple>
        struct HashValueImpl<Tuple, 0>
        {
            static void apply(size_t& seed, Tuple const& tuple)
            {
                hash_combine(seed, get<0>(tuple));
            }
        };
    }

    template <typename ... TT>
    struct hash<std::tuple<TT...>>
    {
        size_t
            operator()(std::tuple<TT...> const& tt) const
        {
            size_t seed = 0;
            HashValueImpl<std::tuple<TT...> >::apply(seed, tt);
            return seed;
        }

    };
}

// A cache keyed by a tuple, and storing TStorage
template <typename TStorage, typename...TKeyPack>
class TupleCache
{
public:
	typedef std::tuple<TKeyPack...> TKey;

    void Clear()
    {
        m_cache.clear();
    }

    TStorage& GetOrCreate(TKeyPack... keys, bool& created)
    {
        TKey key = std::make_tuple(keys...);

        auto it = m_cache.find(key);

        created = (it == m_cache.end());

        if (it == m_cache.end())
        {
            m_cache[key] = TStorage();
            it = m_cache.find(key);
        }

        return it->second;
    }

    const TStorage& Get(TKeyPack... keys, bool& existed) const
    {
        static const TStorage c_invalid = {};

        TKey key = std::make_tuple(keys...);

        auto it = m_cache.find(key);

        if (it == m_cache.end())
        {
            existed = false;
            return c_invalid;
        }

        existed = true;
        return it->second;
    }

    TStorage& Get(TKeyPack... keys, bool& existed)
    {
        static TStorage c_invalid = {};

        TKey key = std::make_tuple(keys...);

        auto it = m_cache.find(key);

        if (it == m_cache.end())
        {
            existed = false;
            return c_invalid;
        }

        existed = true;
        return it->second;
    }

    TStorage& GetOrCreate(TKeyPack... keys)
    {
        bool dummy;
        return GetOrCreate(keys..., dummy);
    }

    template <typename LAMBDA>
    void ForEach(const LAMBDA& lambda)
    {
        for (auto& it : m_cache)
            lambda(it.first, it.second);
    }

    template <typename LAMBDA>
    void ForEach(const LAMBDA& lambda) const
    {
        for (const auto& it : m_cache)
            lambda(it.first, it.second);
    }

private:
    std::unordered_map<TKey, TStorage> m_cache;
};