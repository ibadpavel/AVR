#ifndef RANGETEMPLATE_H
#define RANGETEMPLATE_H

#include <QtGlobal>
#include <QVector>
#include <QPointF>
#include <QRectF>

namespace FileIO {

template<typename T>
class RangeTemplate
{
public:
    explicit RangeTemplate(T min = 0, T max = 0);

    void add(T toMin, T toMax);
    void clampByLength(const T length);
    bool contains(T value) const;
    bool contains(const RangeTemplate<T> &other) const;
    void intersect(const RangeTemplate<T> &other);
    bool isIntersects(const RangeTemplate<T> &other) const;
    bool isValid() const;
    void unite(T value);
    void unite(const RangeTemplate<T> &other);

    // operators
    operator QString() const;
    bool operator!=(const RangeTemplate<T> &other) const;
    bool operator==(const RangeTemplate<T> &other) const;
    RangeTemplate<T> &operator*=(T value);
    RangeTemplate<T> &operator+=(T value);
    RangeTemplate<T> &operator-=(T value);
    RangeTemplate<T> &operator/=(T value);
    RangeTemplate<T> operator*(T value) const;
    RangeTemplate<T> operator+(T value) const;
    RangeTemplate<T> operator-(T value) const;
    RangeTemplate<T> operator/(T value) const;

    // getters
    T center() const;
    T length() const;
    T max_() const;
    T min_() const;

    // setters
    void setLength(T value, bool centered = false);
    void setMax(T value);
    void setMin(T value);
    void setMinMax(T min, T max);

    // static
    static RangeTemplate<T> HorizontalRange(const QRect &rect);
    static RangeTemplate<T> HorizontalRange(const QRectF &rect);
    static RangeTemplate<T> VectorRange(const QVector<QPointF> &points);
    static RangeTemplate<T> VerticalRange(const QRect &rect);
    static RangeTemplate<T> VerticalRange(const QRectF &rect);

private:
    T m_min;
    T m_max;
};

using Range = RangeTemplate<qint64>;
using RangeF = RangeTemplate<qreal>;
using RangeU = RangeTemplate<quint64>;

} // namespace

#endif // RANGETEMPLATE_H
