#pragma once
#include <functional>

namespace Util
{
    template<class ...T>
    struct Event
    {
        using Functor = std::function<void(T...)>;

        inline void add(Functor const& _callback)
        {
            m_callbacks.Add(_callback);
        }

        inline void broadcast(T... _p) const
        {
            for (auto const& callback : m_callbacks)
            {
                callback(_p...);
            }
        }

        inline void clean()
        {
            m_callbacks.Empty();
        }

    private:
        TArray<Functor> m_callbacks {};
    };
}