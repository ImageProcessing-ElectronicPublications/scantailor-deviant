/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C)  Joseph Artsimovich <joseph.artsimovich@gmail.com>

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

#include "ThresholdsApplyWidget.h"
#include <assert.h>

namespace output
{

ThresholdsWidget::ThresholdsWidget(QWidget* parent, ThresholdFilter filter)
    : QWidget(parent)
{
    setupUi(this);

    switch (filter)
    {
    case OTSU:
        otsuCheckBox->setChecked(true);
        break;
    case SAUVOLA:
        sauvolaCheckBox->setChecked(true);
        break;
    case WOLF:
        wolfCheckBox->setChecked(true);
        break;
    case GATOS:
        gatosCheckBox->setChecked(true);
        break;
    default:
        assert(!"Unreachable");
        break;
    }
}

std::vector<ThresholdFilter>
ThresholdsWidget::thresholdsChecked() const
{
    std::vector<ThresholdFilter> thresholds;
    thresholds.reserve(thresholdsGroupBox->layout()->count());

    if (otsuCheckBox->isChecked())
    {
        thresholds.push_back(OTSU);
    }
    if (sauvolaCheckBox->isChecked())
    {
        thresholds.push_back(SAUVOLA);
    }
    if (wolfCheckBox->isChecked())
    {
        thresholds.push_back(WOLF);
    }
    if (gatosCheckBox->isChecked())
    {
        thresholds.push_back(GATOS);
    }

    return thresholds;
}

} // namespace output
