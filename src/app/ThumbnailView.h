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
    Q_PROPERTY(QColor selectedBackgroundColor READ selectedBackgroundColor WRITE setSelectedBackgroundColor)
    Q_PROPERTY(QColor targetedBackgroundColor READ targetedBackgroundColor WRITE setTargetedBackgroundColor)
    Q_PROPERTY(QColor leaderBackgroundColor READ leaderBackgroundColor WRITE setLeaderBackgroundColor)
public:
    ThumbnailView(QWidget* parent);

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

        QColor const& selectedBackgroundColor() const
        {
            return m_rOwner.m_selectedBackgroundColor;
        }

        QColor const& targetedBackgroundColor() const
        {
            return m_rOwner.m_targetedBackgroundColor;
        }

        QColor const& leaderBackgroundColor() const
        {
            return m_rOwner.m_leaderBackgroundColor;
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

    QColor const& selectedBackgroundColor() const
    {
        return m_selectedBackgroundColor;
    }

    void setSelectedBackgroundColor(QColor const& color)
    {
        m_selectedBackgroundColor = color;
    }

    QColor const& targetedBackgroundColor() const
    {
        return m_targetedBackgroundColor;
    }

    void setTargetedBackgroundColor(QColor const& color)
    {
        m_targetedBackgroundColor = color;
    }

    QColor const& leaderBackgroundColor() const
    {
        return m_leaderBackgroundColor;
    }

    void setLeaderBackgroundColor(QColor const& color)
    {
        m_leaderBackgroundColor = color;
    }

    QColor m_textColor;
    QColor m_highlightedTextColor;
    QColor m_selectedBackgroundColor;
    QColor m_targetedBackgroundColor;
    QColor m_leaderBackgroundColor;
    QssStyle m_qssStyle;
};

#endif
