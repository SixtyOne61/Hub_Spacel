// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <utility>
#include <algorithm>
#include "Util/UntypeObject.h"


class HUB_SPACEL_API Fail_cast {};

/**
 *
 */
class HUB_SPACEL_API Any
{
public:
    template<class T>
    inline T& unsafe_downcast() noexcept
    {
        return static_cast<UntypeObject<T>*>(m_p)->get();
    }

    template<class T>
    inline T& downcast()
    {
        if (!is_a<T>())
        {
            throw Fail_cast();
        }

        return unsafe_downcast<T>();
    }

    template<class T>
    inline T const& unsafe_downcast() const noexcept
    {
        return static_cast<UntypeObject<T>*>(m_p)->get();
    }

    template<class T>
    inline T const& downcast()
    {
        if (!is_a<T>())
        {
            throw Fail_cast();
        }

        return unsafe_downcast<T>();
    }

    template<class T>
    inline bool is_a() const noexcept
    {
        return m_p ? m_p->get_type() == typeid(T);
    }

    inline bool empty() const noexcept
    {
        return !m_p;
    }

    inline void swap(Any& _obj) noexcept
    {
        std::swap(m_p, _obj.m_p);
    }

    constexpr Any() = default;

    template<class T>
    Any(T const& _val)
        : m_p { new UntypeObject<T>{_val} }
    {}

    Any(Any const& _cpy)
        : m_p { _cpy.empty() ? nullptr : _cpy.m_p->cloner() }
    {}

    Any(Any && _move)
        : m_p { std::move(_move.m_p) }
    { _move.m_p = {}; }

    Any& operator=(Any&& _move)
    {
        delete m_p;
        m_p = std::move(_move.m_p);
        _move.m_p = {};
        return *this;
    }

    Any& operator=(Any const& _cpy)
    {
        Any { _cpy }.swap(*this);
        return *this;
    }

    template<class T>
    Any& operator=(T const& _val)
    {
        Any { _val }.swap(*this);
        return *this;
    }

    ~Any()
    {
        delete m_p;
        m_p = nullptr;
    }

private:
    SClonable * m_p { nullptr };
};

