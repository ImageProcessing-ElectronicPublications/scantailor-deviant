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

#include "ThumbnailView.h"
#include <QApplication>
#include <QPalette>

ThumbnailView::ThumbnailView(QWidget* parent)
    : QGraphicsView(parent)
    , m_textColor(QApplication::palette().color(QPalette::Text))
    , m_highlightedTextColor(QApplication::palette().color(QPalette::HighlightedText))
    , m_selectedBackgroundColor(QApplication::palette().color(QPalette::Highlight))
    , m_targetedBackgroundColor(QApplication::palette().color(QPalette::Highlight).darker(150))
    , m_leaderBackgroundColor(QApplication::palette().color(QPalette::Highlight).lighter(150))
    , m_qssStyle(*this)
{
}
