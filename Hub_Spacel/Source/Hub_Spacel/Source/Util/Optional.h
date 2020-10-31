#pragma once

namespace Util
{
    template<class T>
    class Optional
    {
    public:
        Optional(Optional<T> const&) = delete;
        Optional<T>& operator=(Optional<T> const&) = delete;

        Optional()
            : m_value()
            , m_hasValue(false)
        {

        }

        Optional(Optional<T>&& _mv)
            : m_value(std::move(_mv.m_value))
            , m_hasValue(_mv.m_hasValue)
        {

        }

        Optional<T>& operator=(Optional<T>&& _mv)
        {
            m_hasValue = _mv.m_hasValue;
            m_value = std::move(_mv.m_value);
        }

        Optional<T>& operator=(T _val)
        {
            m_value = _val;
            m_hasValue = true;
        }

        bool hasValue() const
        {
            return m_hasValue;
        }

        T& value()
        {
            return m_value;
        }

        T const& value() const
        {
            return m_value;
        }

        void reset()
        {
            m_value = { };
            m_hasValue = false;
        }

    private:
        T m_value { };
        bool m_hasValue{ false };
    };
}