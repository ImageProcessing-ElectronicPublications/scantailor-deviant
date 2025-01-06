/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2009  Joseph Artsimovich <joseph_a@mail.ru>

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


#ifndef THUMBNAILVIEW_H_
#define THUMBNAILVIEW_H_

#include <QGraphicsView>
#include <QColor>

class ThumbnailView : public QGraphicsView
{
    Q_OBJECT
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(QColor highlightedTextColor READ highlightedTextColor WRITE setHighlightedTextColor)
public:
    ThumbnailView(QWidget* parent)
        : QGraphicsView(parent)
        , m_qssStyle(*this)
    {
    }

    class QssStyle
    {
    public:
        QssStyle(ThumbnailView& owner) : m_rOwner(owner) {}

        QColor const& textColor() const
        {
            return m_rOwner.m_textColor;
        }

        QColor const& highlightedTextColor() const
        {
            return m_rOwner.m_highlightedTextColor;
        }

    private:
        ThumbnailView& m_rOwner;
    };

    QssStyle const* qssStyle() { return &m_qssStyle; }

private:
    QColor const& textColor() const
    {
        return m_textColor;
    }

    void setTextColor(QColor const& color)
    {
        m_textColor = color;
    }

    QColor const& highlightedTextColor() const
    {
        return m_highlightedTextColor;
    }

    void setHighlightedTextColor(QColor const& color)
    {
        m_highlightedTextColor = color;
    }

    QColor m_textColor;
    QColor m_highlightedTextColor;
    QssStyle m_qssStyle;
};

#endif
