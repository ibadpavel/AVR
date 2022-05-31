#include "rangetemplate.h"

namespace FileIO {

template<typename T>
RangeTemplate<T>::RangeTemplate(T min, T max)
    : m_min(min)
    , m_max(max)
{
    if (m_min > m_max) {
        m_min = m_max;
    }
}

template<typename T>
void RangeTemplate<T>::add(T toMin, T toMax)
{
    m_min += toMin;
    m_max += toMax;
}

template<typename T>
void RangeTemplate<T>::clampByLength(const T length)
{
    if (this->length() > length) {
        setLength(length);
    }
}

template<typename T>
bool RangeTemplate<T>::contains(T value) const
{
    return (m_min <= value) && (value <= m_max);
}

template<typename T>
bool RangeTemplate<T>::contains(const RangeTemplate<T> &other) const
{
    return (m_min <= other.m_min) && (other.m_max <= m_max);
}

template<typename T>
void RangeTemplate<T>::intersect(const RangeTemplate<T> &other)
{
    // if the ranges do not intersect, the variable can become invalid, this is a feature
    if (m_min < other.m_min) {
        m_min = other.m_min;
    }
    if (m_max > other.m_max) {
        m_max = other.m_max;
    }
}

template<typename T>
bool RangeTemplate<T>::isIntersects(const RangeTemplate<T> &other) const
{
    return (m_min <= other.m_max) && (other.m_min <= m_max);
}

template<typename T>
bool RangeTemplate<T>::isValid() const
{
    return m_min <= m_max;
}

template<typename T>
void RangeTemplate<T>::unite(T value)
{
    if (m_min > value) {
        m_min = value;
    } else if (m_max < value) {
        m_max = value;
    }
}

template<typename T>
void RangeTemplate<T>::unite(const RangeTemplate<T> &other)
{
    if (m_min > other.m_min) {
        m_min = other.m_min;
    }
    if (m_max < other.m_max) {
        m_max = other.m_max;
    }
}

template<typename T>
RangeTemplate<T>::operator QString() const
{
    return QString("[%1, %2]").arg(m_min).arg(m_max);
}

template<typename T>
bool RangeTemplate<T>::operator!=(const RangeTemplate<T> &other) const
{
    return !(*this == other);
}

template<typename T>
bool RangeTemplate<T>::operator==(const RangeTemplate<T> &other) const
{
    return (m_min == other.m_min) && (m_max == other.m_max);
}

template<typename T>
RangeTemplate<T> &RangeTemplate<T>::operator*=(T value)
{
    m_min *= value;
    m_max *= value;

    return *this;
}

template<typename T>
RangeTemplate<T> &RangeTemplate<T>::operator+=(T value)
{
    m_min += value;
    m_max += value;

    return *this;
}

template<typename T>
RangeTemplate<T> &RangeTemplate<T>::operator-=(T value)
{
    m_min -= value;
    m_max -= value;

    return *this;
}

template<typename T>
RangeTemplate<T> &RangeTemplate<T>::operator/=(T value)
{
    m_min /= value;
    m_max /= value;

    return *this;
}

template<typename T>
RangeTemplate<T> RangeTemplate<T>::operator*(T value) const
{
    RangeTemplate<T> result(*this);
    result *= value;

    return result;
}

template<typename T>
RangeTemplate<T> RangeTemplate<T>::operator+(T value) const
{
    RangeTemplate<T> result(*this);
    result += value;

    return result;
}

template<typename T>
RangeTemplate<T> RangeTemplate<T>::operator-(T value) const
{
    RangeTemplate<T> result(*this);
    result -= value;

    return result;
}

template<typename T>
RangeTemplate<T> RangeTemplate<T>::operator/(T value) const
{
    RangeTemplate<T> result(*this);
    result /= value;

    return result;
}

template<typename T>
T RangeTemplate<T>::center() const
{
    return (m_max + m_min) / 2;
}

template<typename T>
T RangeTemplate<T>::length() const
{
    return m_max - m_min;
}

template<typename T>
T RangeTemplate<T>::max_() const
{
    return m_max;
}

template<typename T>
T RangeTemplate<T>::min_() const
{
    return m_min;
}

template<typename T>
void RangeTemplate<T>::setLength(T value, bool centered)
{
    if (centered) {
        T center = this->center();
        T distance = value / 2;
        setMinMax(center - distance, center + distance);
    } else {
        setMax(m_min + value);
    }
}

template<typename T>
void RangeTemplate<T>::setMax(T value)
{
    m_max = value;
    if (m_min > m_max) {
        m_min = m_max;
    }
}

template<typename T>
void RangeTemplate<T>::setMin(T value)
{
    m_min = value;
    if (m_max < m_min) {
        m_max = m_min;
    }
}

template<typename T>
void RangeTemplate<T>::setMinMax(T min, T max)
{
    setMin(min);
    setMax(max);
}

template<typename T>
RangeTemplate<T> RangeTemplate<T>::HorizontalRange(const QRect &rect)
{
    return RangeTemplate<T>(rect.left(), rect.right());
}

template<typename T>
RangeTemplate<T> RangeTemplate<T>::HorizontalRange(const QRectF &rect)
{
    return RangeTemplate<T>(rect.left(), rect.right());
}

template<typename T>
RangeTemplate<T> RangeTemplate<T>::VectorRange(const QVector<QPointF> &points)
{
    return points.empty() ? RangeTemplate<T>() : RangeTemplate<T>(points.first().x(), points.last().x());
}

template<typename T>
RangeTemplate<T> RangeTemplate<T>::VerticalRange(const QRect &rect)
{
    return RangeTemplate<T>(rect.top(), rect.bottom());
}

template<typename T>
RangeTemplate<T> RangeTemplate<T>::VerticalRange(const QRectF &rect)
{
    return RangeTemplate<T>(rect.top(), rect.bottom());
}

// explicit instantiations
template class RangeTemplate<qint64>;
template class RangeTemplate<qreal>;
template class RangeTemplate<quint64>;

} // namespace
