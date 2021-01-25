// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <type_traits>
#include <functional>
#include <vector>

namespace EnumUtil
{
    // generique count methode, generate assert
    // reopen namespace on your enum declaration for specialisation

    // write count after your enum declaration
    template<typename T,
        std::enable_if_t<std::is_enum<T>::value, bool> = true>
    inline int count() { static_assert(false, "Enum was not generate count methode in namespace EnumUtil\n."); }

    // alias
    using CallbackList = std::vector<std::function<void(void)>>;

    // only enable for T is an enum
    template<typename T,
        std::enable_if_t<std::is_enum<T>::value, bool> = true>
    class EnumCallback
    {
        // enum
        T m_enum{};
        // call back, can be empty
        CallbackList m_callBacks{};

    public:
        EnumCallback() = default;
        EnumCallback(EnumCallback const&) = delete;

        EnumCallback(CallbackList&& _ls)
        {
            m_callBacks = _ls;
        }

        EnumCallback(T const& _a)
        {
            this = _a;
        }

        inline void init(CallbackList&& _ls) { m_callBacks = _ls; }
        inline T const& get() const { return m_enum; }

        EnumCallback<T>& operator=(T const& _a)
        {
            m_enum = _a;
            size_t id = (size_t)m_enum;
            if (id < m_callBacks.size() && m_callBacks[id] != nullptr)
            {
                m_callBacks[id]();
            }
            return *this;
        }
    };

    // overload operator ==
    template<typename T>
    bool operator == (EnumCallback<T> const& _a, EnumCallback<T> const& _b)
    {
        return _a.get() == _b.get();
    }

    // overload operator ==
    template<typename T>
    bool operator == (EnumCallback<T> const& _a, T const& _b)
    {
        return _a.get() == _b;
    }

    // overload operator !=
    template<typename T>
    bool operator != (EnumCallback<T> const& _a, EnumCallback<T> const& _b)
    {
        return _a.get() != _b.get();
    }

    // overload operator ==
    template<typename T>
    bool operator != (EnumCallback<T> const& _a, T const& _b)
    {
        return _a.get() != _b;
    }
}