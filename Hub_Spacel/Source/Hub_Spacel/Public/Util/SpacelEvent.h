#pragma once
#include <functional>

namespace Util
{
    template<typename P1, typename P2>
    struct EventTwoParams
    {
        inline void add(std::function<void(P1, P2)> const& _callback)
        {
            m_callbacks.Add(_callback);
        }

        inline void broadcast(P1 _p1, P2 _p2) const
        {
            for (auto const& callback : m_callbacks)
            {
                callback(_p1, _p2);
            }
        }

        inline void clean()
        {
            m_callbacks.Empty();
        }

    private:
        TArray<std::function<void(P1, P2)>> m_callbacks {};
    };
}