// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Util/Clonable.h"

/**
 * 
 */
 template<class T>
class HUB_SPACEL_API UntypeObject : public SClonable
{

public:
    using value_type = T;
    friend class Any;

private:
    value_type m_val {};

    inline std::type_info const& get_type() const noexcept override
    {
        return typeid(value_type);
    }

public:
    UntypeObject() = default;
    constexpr UntypeObject(value_type const& _val)
        : m_val { _val }
    {}

    inline UntypeObject<T>* cloner() const override
    {
        return new UntypeObject<T>(*this);
    }

    inline constexpr value_type const& get() const
    {
        return m_val;
    }

    inline constexpr value_type& get() noexcept
    {
        return m_val;
    }
};
