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

#include "OptionsWidget.h"
#include "ApplyToDialog.h"
#include "Settings.h"
#include "DistortionType.h"
#include "ScopedIncDec.h"

namespace deskew
{

double const OptionsWidget::MAX_ANGLE = 45.0;

OptionsWidget::OptionsWidget(IntrusivePtr<Settings> const& settings,
                             PageSelectionAccessor const& page_selection_accessor)
    :	m_ptrSettings(settings)
    ,	m_pageParams(Dependencies())
    ,	m_ignoreSignalsFromUiControls(0)
    ,	m_pageSelectionAccessor(page_selection_accessor)
{
    using namespace dewarping;

    ui.setupUi(this);
    setupDistortionTypeButtons();

    // Fov UI.
    connect(
        ui.fovAutoBtn, SIGNAL(toggled(bool)),
        this, SLOT(fovAutoManualModeChanged(bool)));
    connect(
        ui.fovSlider, SIGNAL(valueChanged(int)),
        SLOT(fovSliderMoved(int))
    );
    connect(
        ui.fovSlider, SIGNAL(sliderReleased()),
        SLOT(fovSliderReleased())
    );
    connect(
        ui.fovMinSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(fovMinSpinBoxValueChanged(double))
    );
    connect(
        ui.fovSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(fovSpinBoxValueChanged(double))
    );
    connect(
        ui.fovMaxSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(fovMaxSpinBoxValueChanged(double))
    );

    // Rotation angle UI.
    ui.angleSpinBox->setSuffix(QChar(0x00B0)); // the degree symbol
    ui.angleSpinBox->setRange(-MAX_ANGLE, MAX_ANGLE);
    ui.angleSpinBox->adjustSize();
    setSpinBoxUnknownState();
    connect(
        ui.angleSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(angleSpinBoxValueChanged(double))
    );

    // Depth perception UI.
    ui.depthPerceptionSlider->setMinimum(
        depthPerceptionToSlider(DepthPerception::minValue())
    );
    ui.depthPerceptionSlider->setMaximum(
        depthPerceptionToSlider(DepthPerception::maxValue())
    );
    connect(
        ui.depthPerceptionSlider, SIGNAL(valueChanged(int)),
        SLOT(depthPerceptionSliderMoved(int))
    );
    connect(
        ui.depthPerceptionSlider, SIGNAL(sliderReleased()),
        SLOT(depthPerceptionSliderReleased())
    );
    connect(
        ui.applyDepthPerceptionBtn, SIGNAL(clicked()),
        SLOT(showApplyDepthPerceptionDialog())
    );

    // Distortion type UI.
    connect(
        ui.noDistortionButton, SIGNAL(toggled(bool)),
        SLOT(noDistortionToggled(bool))
    );
    connect(
        ui.rotationDistortionButton, SIGNAL(toggled(bool)),
        SLOT(rotationDistortionToggled(bool))
    );
    connect(
        ui.perspectiveDistortionButton, SIGNAL(toggled(bool)),
        SLOT(perspectiveDistortionToggled(bool))
    );
    connect(
        ui.warpDistortionButton, SIGNAL(toggled(bool)),
        SLOT(warpDistortionToggled(bool))
    );
    connect(
        ui.applyDistortionTypeBtn, SIGNAL(clicked()),
        this, SLOT(showApplyDistortionTypeDialog())
    );

    // Auto / Manual mode.
    connect(ui.autoBtn, SIGNAL(toggled(bool)), this, SLOT(distortionAutoManualModeChanged(bool)));
}

OptionsWidget::~OptionsWidget()
{
}

void
OptionsWidget::showApplyDistortionTypeDialog()
{
    ApplyToDialog* dialog = new ApplyToDialog(
        this, m_pageId, m_pageSelectionAccessor, PageView::IMAGE_VIEW
    );

    dialog->setWindowTitle(tr("Apply Distortion Type"));

    connect(dialog, &ApplyToDialog::accepted, this, [=]() {
        std::vector<PageId> vec = dialog->getPageRangeSelectorWidget().result();
        std::set<PageId> pages(vec.begin(), vec.end());
        if (!dialog->getPageRangeSelectorWidget().allPagesSelected()) {
            distortionTypeAppliedTo(pages);
        }
        else {
            distortionTypeAppliedToAllPages(pages);
        }
        });

    dialog->show();
}

void
OptionsWidget::distortionTypeAppliedTo(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    m_ptrSettings->setDistortionType(pages, m_pageParams.distortionType());

    for (PageId const& page_id: pages)
    {
        emit invalidateThumbnail(page_id);
    }
}

void
OptionsWidget::distortionTypeAppliedToAllPages(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    m_ptrSettings->setDistortionType(pages, m_pageParams.distortionType());

    emit invalidateAllThumbnails();
}

void
OptionsWidget::showApplyDepthPerceptionDialog()
{
    ApplyToDialog* dialog = new ApplyToDialog(
        this, m_pageId, m_pageSelectionAccessor, PageView::IMAGE_VIEW
    );

    dialog->setWindowTitle(tr("Apply Depth Perception"));

    connect(dialog, &ApplyToDialog::accepted, this, [=]() {
        std::vector<PageId> vec = dialog->getPageRangeSelectorWidget().result();
        std::set<PageId> pages(vec.begin(), vec.end());
        if (!dialog->getPageRangeSelectorWidget().allPagesSelected()) {
            depthPerceptionAppliedTo(pages);
        }
        else {
            depthPerceptionAppliedToAllPages(pages);
        }
        });

    dialog->show();
}

void
OptionsWidget::depthPerceptionAppliedTo(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    m_ptrSettings->setDepthPerception(pages, m_pageParams.dewarpingParams().depthPerception());

    for (PageId const& page_id : pages)
    {
        emit invalidateThumbnail(page_id);
    }
}

void
OptionsWidget::depthPerceptionAppliedToAllPages(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    m_ptrSettings->setDepthPerception(pages, m_pageParams.dewarpingParams().depthPerception());
    emit invalidateAllThumbnails();
}

void
OptionsWidget::manualDistortionModelSetExternally(
    dewarping::DistortionModel const& model)
{
    // As we reuse DewarpingView for DistortionType::PERSPECTIVE,
    // we get called both in dewarping and perspective modes.
    if (m_pageParams.distortionType() == DistortionType::WARP)
    {
        m_pageParams.dewarpingParams().setDistortionModel(model);
        m_pageParams.dewarpingParams().setMode(MODE_MANUAL);
    }
    else if (m_pageParams.distortionType() == DistortionType::PERSPECTIVE)
    {
        m_pageParams.perspectiveParams().setCorner(
            PerspectiveParams::TOP_LEFT, model.topCurve().polyline().front()
        );
        m_pageParams.perspectiveParams().setCorner(
            PerspectiveParams::TOP_RIGHT, model.topCurve().polyline().back()
        );
        m_pageParams.perspectiveParams().setCorner(
            PerspectiveParams::BOTTOM_LEFT, model.bottomCurve().polyline().front()
        );
        m_pageParams.perspectiveParams().setCorner(
            PerspectiveParams::BOTTOM_RIGHT, model.bottomCurve().polyline().back()
        );
        m_pageParams.perspectiveParams().setMode(MODE_MANUAL);
    }
    else
    {
        assert(!"unreachable");
    }

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);
    updateModeIndication(MODE_MANUAL);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::manualDeskewAngleSetExternally(double const degrees)
{
    m_pageParams.rotationParams().setCompensationAngleDeg(degrees);
    m_pageParams.rotationParams().setMode(MODE_MANUAL);
    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    updateModeIndication(MODE_MANUAL);
    setSpinBoxKnownState(degreesToSpinBox(degrees));

    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::preUpdateUI(PageId const& page_id, DistortionType const& distortion_type)
{
    ScopedIncDec<int> guard(m_ignoreSignalsFromUiControls);

    m_pageId = page_id;
    m_pageParams.setDistortionType(distortion_type);

    switch (distortion_type)
    {
    case DistortionType::PERSPECTIVE:
        updateFovPanel(m_pageParams.perspectiveParams().fovParams());
        updateFramePanel(m_pageParams.perspectiveParams().frameParams());
        updateSizePanel(m_pageParams.perspectiveParams().sizeParams());
        updateMarginsPanel(m_pageParams.perspectiveParams().marginsParams());
        break;
    case DistortionType::WARP:
        updateFovPanel(m_pageParams.dewarpingParams().fovParams());
        updateFramePanel(m_pageParams.dewarpingParams().frameParams());
        updateBendPanel(m_pageParams.dewarpingParams().bendParams());
        updateSizePanel(m_pageParams.dewarpingParams().sizeParams());
        updateMarginsPanel(m_pageParams.dewarpingParams().marginsParams());
        break;
    }

    setupUiForDistortionType(distortion_type);
    disableDistortionDependentUiElements();
}

void
OptionsWidget::postUpdateUI(Params const& page_params)
{
    ScopedIncDec<int> guard(m_ignoreSignalsFromUiControls);

    m_pageParams = page_params;
    ui.autoBtn->setEnabled(true);
    ui.manualBtn->setEnabled(true);
    updateModeIndication(page_params.mode());

    switch (page_params.distortionType())
    {
    case DistortionType::PERSPECTIVE:
        updateFovPanel(page_params.perspectiveParams().fovParams());
        updateFramePanel(page_params.perspectiveParams().frameParams());
        updateSizePanel(page_params.perspectiveParams().sizeParams());
        updateMarginsPanel(page_params.perspectiveParams().marginsParams());
        break;
    case DistortionType::WARP:
        updateFovPanel(page_params.dewarpingParams().fovParams());
        updateFramePanel(page_params.dewarpingParams().frameParams());
        updateBendPanel(page_params.dewarpingParams().bendParams());
        updateSizePanel(page_params.dewarpingParams().sizeParams());
        updateMarginsPanel(page_params.dewarpingParams().marginsParams());
        break;
    }

    enableDistortionDependentUiElements();

    if (page_params.distortionType() == DistortionType::ROTATION)
    {
        double const angle = page_params.rotationParams().compensationAngleDeg();
        setSpinBoxKnownState(degreesToSpinBox(angle));
    }
    else if (page_params.distortionType() == DistortionType::WARP)
    {
        double const depth_perception = page_params.dewarpingParams().depthPerception().value();
        ui.depthPerceptionSlider->setValue(depthPerceptionToSlider(depth_perception));
    }
}

void
OptionsWidget::noDistortionToggled(bool checked)
{
    if (!checked || m_ignoreSignalsFromUiControls)
    {
        return;
    }

    m_pageParams.setDistortionType(DistortionType::NONE);
    m_ptrSettings->setPageParams(m_pageId, m_pageParams);
    setupUiForDistortionType(DistortionType::NONE);
    emit reloadRequested();
}

void
OptionsWidget::rotationDistortionToggled(bool checked)
{
    if (!checked || m_ignoreSignalsFromUiControls)
    {
        return;
    }

    m_pageParams.setDistortionType(DistortionType::ROTATION);
    m_ptrSettings->setPageParams(m_pageId, m_pageParams);
    setupUiForDistortionType(DistortionType::ROTATION);
    emit reloadRequested();
}

void
OptionsWidget::perspectiveDistortionToggled(bool checked)
{
    if (!checked || m_ignoreSignalsFromUiControls)
    {
        return;
    }

    m_pageParams.setDistortionType(DistortionType::PERSPECTIVE);
    m_ptrSettings->setPageParams(m_pageId, m_pageParams);
    setupUiForDistortionType(DistortionType::PERSPECTIVE);
    emit reloadRequested();
}

void
OptionsWidget::warpDistortionToggled(bool checked)
{
    if (!checked || m_ignoreSignalsFromUiControls)
    {
        return;
    }

    m_pageParams.setDistortionType(DistortionType::WARP);
    m_ptrSettings->setPageParams(m_pageId, m_pageParams);
    setupUiForDistortionType(DistortionType::WARP);
    emit reloadRequested();
}

void
OptionsWidget::angleSpinBoxValueChanged(double const value)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    double const degrees = spinBoxToDegrees(value);
    m_pageParams.rotationParams().setCompensationAngleDeg(degrees);
    m_pageParams.rotationParams().setMode(MODE_MANUAL);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);
    updateModeIndication(MODE_MANUAL);

    emit manualDeskewAngleSet(degrees);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::distortionAutoManualModeChanged(bool const auto_mode)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    switch (m_pageParams.distortionType().get())
    {
    case DistortionType::NONE:
        break;
    case DistortionType::ROTATION:
        if (auto_mode)
        {
            m_pageParams.rotationParams().invalidate();
        }
        else
        {
            m_pageParams.rotationParams().setMode(MODE_MANUAL);
        }
        break;
    case DistortionType::PERSPECTIVE:
        if (auto_mode)
        {
            m_pageParams.perspectiveParams().invalidate();
        }
        else
        {
            m_pageParams.perspectiveParams().setMode(MODE_MANUAL);
        }
        break;
    case DistortionType::WARP:
        if (auto_mode)
        {
            m_pageParams.dewarpingParams().invalidate();
        }
        else
        {
            m_pageParams.dewarpingParams().setMode(MODE_MANUAL);
        }
        break;
    }

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);
    if (auto_mode)
    {
        emit reloadRequested();
    }
}

void
OptionsWidget::depthPerceptionSliderMoved(int value)
{
    double const depth_perception = sliderToDepthPerception(value);
    m_pageParams.dewarpingParams().setDepthPerception(depth_perception);
    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    emit depthPerceptionSetByUser(depth_perception);

    if (!ui.depthPerceptionSlider->isSliderDown())
    {
        emit invalidateThumbnail(m_pageId);
    }
}

void
OptionsWidget::depthPerceptionSliderReleased()
{
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::fovAutoManualModeChanged(bool auto_mode)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::FovParams& fov_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().fovParams() :
        m_pageParams.dewarpingParams().fovParams();

    if (auto_mode)
    {
        fov_params.setMode(MODE_AUTO);
        ui.fovSlider->setDisabled(true);
        ui.fovSpinBox->setDisabled(true);
    }
    else
    {
        fov_params.setMode(MODE_MANUAL);
        ui.fovSlider->setEnabled(true);
        ui.fovSpinBox->setEnabled(true);
    }

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    if (auto_mode)
    {
        emit reloadRequested();
    }
}

void
OptionsWidget::fovSliderMoved(int value)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    double const fov = sliderToFov(value);

    dewarping::FovParams& fov_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().fovParams() :
        m_pageParams.dewarpingParams().fovParams();

    fov_params.setFov(fov);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);
    
    ui.fovSpinBox->setValue(fov);

    emit fovParamsSetByUser(fov_params);

    if (!ui.fovSlider->isSliderDown())
    {
        emit invalidateThumbnail(m_pageId);
    }
}

void
OptionsWidget::fovSliderReleased()
{
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::fovMinSpinBoxValueChanged(double fov_min_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::FovParams& fov_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().fovParams() :
        m_pageParams.dewarpingParams().fovParams();

    if (fov_params.fov() < fov_min_new)
    {
        fov_params.setFov(fov_min_new);
    }

    fov_params.setFovMin(fov_min_new);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    ui.fovSlider->setMinimum(fovToSlider(fov_min_new));
    ui.fovSpinBox->setMinimum(fov_min_new);
    ui.fovMaxSpinBox->setMinimum(fov_min_new);

    emit fovParamsSetByUser(fov_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::fovSpinBoxValueChanged(double fov_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::FovParams& fov_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().fovParams() :
        m_pageParams.dewarpingParams().fovParams();

    fov_params.setFov(fov_new);
    
    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    ui.fovSlider->setValue(fovToSlider(fov_new));

    emit fovParamsSetByUser(fov_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::fovMaxSpinBoxValueChanged(double fov_max_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::FovParams& fov_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().fovParams() :
        m_pageParams.dewarpingParams().fovParams();

    if (fov_params.fov() > fov_max_new)
    {
        fov_params.setFov(fov_max_new);
    }

    fov_params.setFovMax(fov_max_new);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    ui.fovSlider->setMaximum(fovToSlider(fov_max_new));
    ui.fovSpinBox->setMaximum(fov_max_new);
    ui.fovMinSpinBox->setMaximum(fov_max_new);

    emit fovParamsSetByUser(fov_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::setupDistortionTypeButtons()
{
    static_assert(
        DistortionType::LAST + 1 - DistortionType::FIRST == 4,
        "Unexpected number of distortion types"
    );
    m_distortionTypeButtons[DistortionType::NONE] = ui.noDistortionButton;
    m_distortionTypeButtons[DistortionType::ROTATION] = ui.rotationDistortionButton;
    m_distortionTypeButtons[DistortionType::PERSPECTIVE] = ui.perspectiveDistortionButton;
    m_distortionTypeButtons[DistortionType::WARP] = ui.warpDistortionButton;
}

void
OptionsWidget::disableDistortionDependentUiElements()
{
    ui.autoManualPanel->setDisabled(true);
    ui.rotationPanel->setDisabled(true);
    ui.depthPerceptionPanel->setDisabled(true);
    ui.fovPanel->setDisabled(true);
    ui.framePanel->setDisabled(true);
    ui.bendPanel->setDisabled(true);
    ui.sizePanel->setDisabled(true);
    ui.marginsPanel->setDisabled(true);
}

void
OptionsWidget::enableDistortionDependentUiElements()
{
    ui.autoManualPanel->setEnabled(true);
    ui.rotationPanel->setEnabled(true);
    ui.depthPerceptionPanel->setEnabled(true);
    ui.fovPanel->setEnabled(true);
    ui.framePanel->setEnabled(true);
    ui.bendPanel->setEnabled(true);
    ui.sizePanel->setEnabled(true);
    ui.marginsPanel->setEnabled(true);
}

void
OptionsWidget::setupUiForDistortionType(DistortionType::Type type)
{
    ScopedIncDec<int> guard(m_ignoreSignalsFromUiControls);

    m_distortionTypeButtons[type]->setChecked(true);

    ui.autoManualPanel->setVisible(type != DistortionType::NONE);
    ui.rotationPanel->setVisible(type == DistortionType::ROTATION);
    ui.depthPerceptionPanel->setVisible(type == DistortionType::WARP);
    ui.fovPanel->setVisible(type == DistortionType::PERSPECTIVE || type == DistortionType::WARP);
    ui.framePanel->setVisible(type == DistortionType::PERSPECTIVE || type == DistortionType::WARP);
    ui.bendPanel->setVisible(type == DistortionType::WARP);
    ui.sizePanel->setVisible(type == DistortionType::PERSPECTIVE || type == DistortionType::WARP);
    ui.marginsPanel->setVisible(type == DistortionType::PERSPECTIVE || type == DistortionType::WARP);
}

void
OptionsWidget::updateModeIndication(AutoManualMode const mode)
{
    ScopedIncDec<int> guard(m_ignoreSignalsFromUiControls);

    if (mode == MODE_AUTO)
    {
        ui.autoBtn->setChecked(true);
    }
    else
    {
        ui.manualBtn->setChecked(true);
    }
}

void
OptionsWidget::updateFovPanel(dewarping::FovParams const& fov_params)
{
    if (fov_params.mode() == MODE_AUTO)
    {
        ui.fovAutoBtn->setChecked(true);
        ui.fovSlider->setDisabled(true);
        ui.fovSpinBox->setDisabled(true);
    }
    else
    {
        ui.fovManualBtn->setChecked(true);
        ui.fovSlider->setEnabled(true);
        ui.fovSpinBox->setEnabled(true);
    }

    ui.fovSlider->setRange(
        fovToSlider(fov_params.fovMin()),
        fovToSlider(fov_params.fovMax()));
    ui.fovSlider->setValue(fovToSlider(fov_params.fov()));

    ui.fovMinSpinBox->setValue(fov_params.fovMin());
    ui.fovSpinBox->setValue(fov_params.fov());
    ui.fovMaxSpinBox->setValue(fov_params.fovMax());

    ui.fovMinSpinBox->setMaximum(fov_params.fovMax());
    ui.fovMaxSpinBox->setMinimum(fov_params.fovMin());
    ui.fovSpinBox->setRange(
        fov_params.fovMin(), 
        fov_params.fovMax());
}

void
OptionsWidget::updateFramePanel(dewarping::FrameParams const& frame_params)
{
    if (frame_params.mode() == MODE_AUTO)
    {
        ui.frameAutoBtn->setChecked(true);
        ui.frameWidthSpinBox->setDisabled(true);
        ui.frameHeightSpinBox->setDisabled(true);
        ui.frameCenterXSpinBox->setDisabled(true);
        ui.frameCenterYSpinBox->setDisabled(true);
    }
    else
    {
        ui.frameManualBtn->setChecked(true);
        ui.frameWidthSpinBox->setEnabled(true);
        ui.frameHeightSpinBox->setEnabled(true);
        ui.frameCenterXSpinBox->setEnabled(true);
        ui.frameCenterYSpinBox->setEnabled(true);
    }

    ui.frameWidthSpinBox->setValue(frame_params.width());
    ui.frameHeightSpinBox->setValue(frame_params.height());
    ui.frameCenterXSpinBox->setValue(frame_params.centerX());
    ui.frameCenterYSpinBox->setValue(frame_params.centerY());
}

void
OptionsWidget::updateBendPanel(dewarping::BendParams const& bend_params)
{
    if (bend_params.mode() == MODE_AUTO)
    {
        ui.bendAutoBtn->setChecked(true);
        ui.bendHorizontalSlider->setDisabled(true);
        ui.bendSpinBox->setDisabled(true);
    }
    else
    {
        ui.bendManualBtn->setChecked(true);
        ui.bendHorizontalSlider->setEnabled(true);
        ui.bendSpinBox->setEnabled(true);
    }

    ui.bendHorizontalSlider->setRange(
        bendToSlider(bend_params.bendMin()),
        bendToSlider(bend_params.bendMax())
    );
    ui.bendHorizontalSlider->setValue(bendToSlider(bend_params.bend()));

    ui.bendMinSpinBox->setValue(bend_params.bendMin());
    ui.bendSpinBox->setValue(bend_params.bend());
    ui.bendMaxSpinBox->setValue(bend_params.bendMax());
}

void
OptionsWidget::updateSizePanel(dewarping::SizeParams const& size_params)
{
    switch (ui.sizeModeComboBox->currentIndex())
    {
    case dewarping::SizeMode::CALC_BY_AREA:
        ui.widthSpinBox->setDisabled(true);
        ui.heightSpinBox->setDisabled(true);
        ui.distanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::FIT_WIDTH:
        ui.widthSpinBox->setEnabled(true);
        ui.heightSpinBox->setDisabled(true);
        ui.distanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::FIT_HEIGHT:
        ui.widthSpinBox->setDisabled(true);
        ui.heightSpinBox->setEnabled(true);
        ui.distanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::STRETCH_TO:
        ui.widthSpinBox->setEnabled(true);
        ui.heightSpinBox->setEnabled(true);
        ui.distanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::CALC_BY_DISTANCE:
        ui.widthSpinBox->setDisabled(true);
        ui.heightSpinBox->setDisabled(true);
        ui.distanceSpinBox->setEnabled(true);
        break;
    }

    ui.widthSpinBox->setValue(size_params.width());
    ui.heightSpinBox->setValue(size_params.height());
    ui.distanceSpinBox->setValue(size_params.distance());
}

void
OptionsWidget::updateMarginsPanel(dewarping::MarginsParams const& margins_params)
{
    ui.marginsLeftSpinBox->setValue(margins_params.left());
    ui.marginsRightSpinBox->setValue(margins_params.right());
    ui.marginsTopSpinBox->setValue(margins_params.top());
    ui.marginsBottomSpinBox->setValue(margins_params.bottom());
    ui.marginsPixelScaleSpinBox->setValue(margins_params.maxPixelScale());
}

void
OptionsWidget::setSpinBoxUnknownState()
{
    ScopedIncDec<int> guard(m_ignoreSignalsFromUiControls);

    ui.angleSpinBox->setSpecialValueText("?");
    ui.angleSpinBox->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    ui.angleSpinBox->setValue(ui.angleSpinBox->minimum());
    ui.angleSpinBox->setEnabled(false);
}

void
OptionsWidget::setSpinBoxKnownState(double const angle)
{
    ScopedIncDec<int> guard(m_ignoreSignalsFromUiControls);

    ui.angleSpinBox->setSpecialValueText("");
    ui.angleSpinBox->setValue(angle);

    // Right alignment doesn't work correctly, so we use the left one.
    ui.angleSpinBox->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    ui.angleSpinBox->setEnabled(true);
}

double
OptionsWidget::spinBoxToDegrees(double const sb_value)
{
    // The spin box shows the angle in a usual geometric way,
    // with positive angles going counter-clockwise.
    // Internally, we operate with angles going clockwise,
    // because the Y axis points downwards in computer graphics.
    return -sb_value;
}

double
OptionsWidget::degreesToSpinBox(double const degrees)
{
    // See above.
    return -degrees;
}

int
OptionsWidget::depthPerceptionToSlider(double depth_perception)
{
    return qRound(depth_perception * 10);
}

double
OptionsWidget::sliderToDepthPerception(int slider_value)
{
    return slider_value / 10.0;
}

int
OptionsWidget::fovToSlider(double fov)
{
    return qRound(fov * 1000);
}

double
OptionsWidget::sliderToFov(int slider_value)
{
    return slider_value / 1000.0;
}

int
OptionsWidget::bendToSlider(double bend)
{
    return qRound(bend * 1000);
}

double
OptionsWidget::sliderToBend(int slider_value)
{
    return slider_value / 1000.0;
}

} // namespace deskew
