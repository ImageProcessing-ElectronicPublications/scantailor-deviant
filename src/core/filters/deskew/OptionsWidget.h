/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2007-2008  Joseph Artsimovich <joseph_a@mail.ru>

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

#ifndef DESKEW_OPTIONSWIDGET_H_
#define DESKEW_OPTIONSWIDGET_H_

#include "ui_DeskewOptionsWidget.h"
#include "FilterOptionsWidget.h"
#include "DistortionType.h"
#include "IntrusivePtr.h"
#include "Params.h"
#include "AutoManualMode.h"
#include "PageSelectionAccessor.h"

namespace dewarping
{
class DistortionModel;
}

namespace deskew
{

class DistortionType;
class Settings;

class OptionsWidget : public FilterOptionsWidget
{
    Q_OBJECT
public:
    OptionsWidget(IntrusivePtr<Settings> const& settings,
                  PageSelectionAccessor const& page_selection_accessor);

    virtual ~OptionsWidget();
signals:
    void manualDeskewAngleSet(double degrees);

    void depthPerceptionSetByUser(double depth_perception);
public slots:
    void manualDeskewAngleSetExternally(double degrees);

    void manualDistortionModelSetExternally(
        dewarping::DistortionModel const& model);
public:
    void preUpdateUI(PageId const& page_id, DistortionType const& distortion_type);

    void postUpdateUI(Params const& page_params);
private slots:
    void noDistortionToggled(bool checked);

    void rotationDistortionToggled(bool checked);

    void perspectiveDistortionToggled(bool checked);

    void warpDistortionToggled(bool checked);

    void angleSpinBoxValueChanged(double skew_degrees);

    void distortionAutoManualModeChanged(bool auto_mode);

    void depthPerceptionSliderMoved(int value);

    void depthPerceptionSliderReleased();

    void showApplyDistortionTypeDialog();

    void distortionTypeAppliedTo(std::set<PageId> const& pages);

    void distortionTypeAppliedToAllPages(std::set<PageId> const& pages);

    void showApplyDepthPerceptionDialog();

    void depthPerceptionAppliedTo(std::set<PageId> const& pages);

    void depthPerceptionAppliedToAllPages(std::set<PageId> const& pages);
private:
    void setupDistortionTypeButtons();

    void disableDistortionDependentUiElements();

    void enableDistortionDependentUiElements();

    void setupUiForDistortionType(DistortionType::Type type);

    void updateModeIndication(AutoManualMode mode);

    void updateFovPanel(dewarping::FovParams const& fov_params);

    void updateFramePanel(dewarping::FrameParams const& frame_params);

    void updateBendPanel(dewarping::BendParams const& bend_params);

    void updateSizePanel(dewarping::SizeParams const& size_params);

    void setSpinBoxUnknownState();

    void setSpinBoxKnownState(double angle);

    static double spinBoxToDegrees(double sb_value);

    static double degreesToSpinBox(double degrees);

    static int depthPerceptionToSlider(double depth_perception);

    static double sliderToDepthPerception(int slider_value);

    static int fovToSlider(double fov);

    static double sliderToFov(int slider_value);

    static int bendToSlider(double bend);

    static double sliderToBend(int slider_value);

    static double const MAX_ANGLE;

    Ui::DeskewOptionsWidget ui;
    IntrusivePtr<Settings> m_ptrSettings;
    PageId m_pageId;

    /**
     * m_pageParams is not always up to date. We make sure not to commit
     * it to m_ptrSettings between preUpdateUI() and postUpdateUI(),
     * where it's certainly not up to date.
     */
    Params m_pageParams;

    QAbstractButton* m_distortionTypeButtons[DistortionType::LAST + 1];
    int m_ignoreSignalsFromUiControls;

    PageSelectionAccessor m_pageSelectionAccessor;
};

} // namespace deskew

#endif
