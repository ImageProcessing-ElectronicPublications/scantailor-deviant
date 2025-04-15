/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2015  Joseph Artsimovich <joseph.artsimovich@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DESKEW_VALUE_H_
#define DESKEW_VALUE_H_

class Value
{
public:
    Value()
        : m_valid(false)
    {
    }

    Value(double value)
        : m_value(value)
        , m_valid(true)
    {
    }

    void operator=(double value)
    {
        m_value = value;
        m_valid = true;
    }

    operator double() const
    {
        return m_value;
    }

    double toDouble() const
    {
        return m_value;
    }

    bool isValid() const
    {
        return m_valid;
    }

    void invalidate()
    {
        m_valid = false;
    }

private:
    double m_value;
    bool m_valid;
};

#endif
