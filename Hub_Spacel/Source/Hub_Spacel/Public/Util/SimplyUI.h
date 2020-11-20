// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <tuple>

namespace SimplyUI
{
	struct FVisibility
	{
		FVisibility(FVisibility&&) = default;
		FVisibility& operator=(FVisibility &&) = default;

		ESlateVisibility m_visibility {};

        template<class ... Ts>
        void read(std::tuple<Ts...> _tuples)
        {
            apply(_tuples, std::make_index_sequence<sizeof...(Ts)>());
        }

        template<class ... Ts, size_t ... Ns>
        void apply(std::tuple<Ts...> _tuples, std::index_sequence<Ns...>)
        {
            apply_impl(std::get<Ns>(_tuples)...);
        }

        template<class T, class ... Ts>
        void apply_impl(T&& _t, Ts&& ... _tuples)
        {
            _t->SetVisibility(m_visibility);
            apply_impl(std::forward<Ts>(_tuples)...);
        }

        template<class T>
        void apply_impl(T&& _t)
        {
            _t->SetVisibility(m_visibility);
        }
	};

	template<class ... Ts>
	static void setVisibility(FVisibility&& _obj, std::tuple<Ts...> _tuples)
	{
		_obj.read(_tuples);
	}

    template<class T, class U>
    static inline U* initSafetyFromName(T* _owner, const FName& _name)
    {
        U* obj = (U*)_owner->GetWidgetFromName(_name);
        if (!ensure(obj != nullptr)) return nullptr;
        return obj;
    }
}

