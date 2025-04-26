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
#include "dewarping/CylindricalSurfaceDewarper.h"
#include <algorithm>

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
    connect(
        ui.fovApplyBtn, SIGNAL(clicked()),
        this, SLOT(showApplyFovParamsDialog())
    );

    // Frame UI.
    connect(
        ui.frameAutoBtn, SIGNAL(toggled(bool)),
        this, SLOT(frameAutoManualModeChanged(bool)));
    connect(
        ui.frameWidthSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(frameWidthSpinBoxValueChanged(double)));
    connect(
        ui.frameHeightSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(frameHeightSpinBoxValueChanged(double)));
    connect(
        ui.frameCenterXSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(frameCenterXSpinBoxValueChanged(double)));
    connect(
        ui.frameCenterYSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(frameCenterYSpinBoxValueChanged(double)));
    connect(
        ui.frameApplyBtn, SIGNAL(clicked()),
        this, SLOT(showApplyFrameParamsDialog())
    );

    // Bend UI.
    connect(
        ui.bendAutoBtn, SIGNAL(toggled(bool)),
        this, SLOT(bendAutoManualModeChanged(bool)));
    connect(
        ui.bendSlider, SIGNAL(valueChanged(int)),
        SLOT(bendSliderMoved(int))
    );
    connect(
        ui.bendSlider, SIGNAL(sliderReleased()),
        SLOT(bendSliderReleased())
    );
    connect(
        ui.bendMinSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(bendMinSpinBoxValueChanged(double))
    );
    connect(
        ui.bendSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(bendSpinBoxValueChanged(double))
    );
    connect(
        ui.bendMaxSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(bendMaxSpinBoxValueChanged(double))
    );
    connect(
        ui.bendApplyBtn, SIGNAL(clicked()),
        this, SLOT(showApplyBendParamsDialog())
    );

    // Size UI.
    connect(
        ui.sizeModeComboBox,SIGNAL(currentIndexChanged(int)),
        this, SLOT(sizeModeComboBoxIndexChanged(int))
    );
    connect(
        ui.sizeWidthSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(sizeWidthSpinBoxValueChanged(double))
    );
    connect(
        ui.sizeHeightSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(sizeHeightSpinBoxValueChanged(double))
    );
    connect(
        ui.sizeDistanceSpinBox, SIGNAL(valueChanged(double)),
        this, SLOT(sizeDistanceSpinBoxValueChanged(double))
    );
    connect(
        ui.sizeApplyBtn, SIGNAL(clicked()),
        this, SLOT(showApplySizeParamsDialog())
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
    connect(
        ui.modeAutoBtn, SIGNAL(toggled(bool)),
        this, SLOT(modeChanged(bool)));
    connect(
        ui.modeApplyBtn, SIGNAL(clicked()),
        this, SLOT(showApplyModeDialog())
    );
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
OptionsWidget::showApplyModeDialog()
{
    ApplyToDialog* dialog = new ApplyToDialog(
        this, m_pageId, m_pageSelectionAccessor, PageView::IMAGE_VIEW
    );

    dialog->setWindowTitle(tr("Apply Auto/Manual mode"));

    connect(dialog, &ApplyToDialog::accepted, this, [=]() {
        std::vector<PageId> vec = dialog->getPageRangeSelectorWidget().result();
        std::set<PageId> pages(vec.begin(), vec.end());
        if (!dialog->getPageRangeSelectorWidget().allPagesSelected()) {
            modeAppliedTo(pages);
        }
        else {
            modeAppliedToAllPages(pages);
        }
        });

    dialog->show();
}

void
OptionsWidget::showApplyFovParamsDialog()
{
    ApplyToDialog* dialog = new ApplyToDialog(
        this, m_pageId, m_pageSelectionAccessor, PageView::IMAGE_VIEW
    );

    dialog->setWindowTitle(tr("Apply Fov Parameters"));

    connect(dialog, &ApplyToDialog::accepted, this, [=]() {
        std::vector<PageId> vec = dialog->getPageRangeSelectorWidget().result();
        std::set<PageId> pages(vec.begin(), vec.end());
        if (!dialog->getPageRangeSelectorWidget().allPagesSelected()) {
            fovParamsAppliedTo(pages);
        }
        else {
            fovParamsAppliedToAllPages(pages);
        }
        });

    dialog->show();
}

void
OptionsWidget::showApplyFrameParamsDialog()
{
    ApplyToDialog* dialog = new ApplyToDialog(
        this, m_pageId, m_pageSelectionAccessor, PageView::IMAGE_VIEW
    );

    dialog->setWindowTitle(tr("Apply Frame Parameters"));

    connect(dialog, &ApplyToDialog::accepted, this, [=]() {
        std::vector<PageId> vec = dialog->getPageRangeSelectorWidget().result();
        std::set<PageId> pages(vec.begin(), vec.end());
        if (!dialog->getPageRangeSelectorWidget().allPagesSelected()) {
            frameParamsAppliedTo(pages);
        }
        else {
            frameParamsAppliedToAllPages(pages);
        }
        });

    dialog->show();
}

void
OptionsWidget::showApplyBendParamsDialog()
{
    ApplyToDialog* dialog = new ApplyToDialog(
        this, m_pageId, m_pageSelectionAccessor, PageView::IMAGE_VIEW
    );

    dialog->setWindowTitle(tr("Apply Bend Parameters"));

    connect(dialog, &ApplyToDialog::accepted, this, [=]() {
        std::vector<PageId> vec = dialog->getPageRangeSelectorWidget().result();
        std::set<PageId> pages(vec.begin(), vec.end());
        if (!dialog->getPageRangeSelectorWidget().allPagesSelected()) {
            bendParamsAppliedTo(pages);
        }
        else {
            bendParamsAppliedToAllPages(pages);
        }
        });

    dialog->show();
}

void
OptionsWidget::showApplySizeParamsDialog()
{
    ApplyToDialog* dialog = new ApplyToDialog(
        this, m_pageId, m_pageSelectionAccessor, PageView::IMAGE_VIEW
    );

    dialog->setWindowTitle(tr("Apply Size Parameters"));

    connect(dialog, &ApplyToDialog::accepted, this, [=]() {
        std::vector<PageId> vec = dialog->getPageRangeSelectorWidget().result();
        std::set<PageId> pages(vec.begin(), vec.end());
        if (!dialog->getPageRangeSelectorWidget().allPagesSelected()) {
            sizeParamsAppliedTo(pages);
        }
        else {
            sizeParamsAppliedToAllPages(pages);
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
OptionsWidget::modeAppliedTo(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    switch (m_pageParams.distortionType())
    {
    case DistortionType::ROTATION:
        m_ptrSettings->setRotationMode(pages, m_pageParams.rotationParams().mode());
        break;
    case DistortionType::PERSPECTIVE:
        m_ptrSettings->setPerspectiveMode(pages, m_pageParams.perspectiveParams().mode());
        break;
    case DistortionType::WARP:
        m_ptrSettings->setDewarpingMode(pages, m_pageParams.dewarpingParams().mode());
        break;
    default:
        break;
    }

    for (PageId const& page_id : pages)
    {
        emit invalidateThumbnail(page_id);
    }
}

void
OptionsWidget::modeAppliedToAllPages(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    switch (m_pageParams.distortionType())
    {
    case DistortionType::ROTATION:
        m_ptrSettings->setRotationMode(pages, m_pageParams.rotationParams().mode());
        break;
    case DistortionType::PERSPECTIVE:
        m_ptrSettings->setPerspectiveMode(pages, m_pageParams.perspectiveParams().mode());
        break;
    case DistortionType::WARP:
        m_ptrSettings->setDewarpingMode(pages, m_pageParams.dewarpingParams().mode());
        break;
    default:
        break;
    }

    emit invalidateAllThumbnails();
}

void
OptionsWidget::fovParamsAppliedTo(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    std::set<PageId>  pages_to_apply;
    std::copy_if(
        pages.begin(),
        pages.end(),
        std::inserter(pages_to_apply, pages_to_apply.end()),
        [this](PageId const& page_id)
        {
            return this->m_pageId != page_id;
        }
    );

    switch (m_pageParams.distortionType())
    {
    case DistortionType::PERSPECTIVE:
        m_ptrSettings->setPerspectiveFovParams(pages_to_apply, m_pageParams.perspectiveParams().fovParams());
        break;
    case DistortionType::WARP:
        m_ptrSettings->setDewarpingFovParams(pages_to_apply, m_pageParams.dewarpingParams().fovParams());
        break;
    default:
        break;
    }

    for (PageId const& page_id : pages_to_apply)
    {
        emit invalidateThumbnail(page_id);
    }
}

void
OptionsWidget::fovParamsAppliedToAllPages(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    std::set<PageId>  pages_to_apply;
    std::copy_if(
        pages.begin(),
        pages.end(),
        std::inserter(pages_to_apply, pages_to_apply.end()),
        [this](PageId const& page_id)
        {
            return this->m_pageId != page_id;
        }
    );

    switch (m_pageParams.distortionType())
    {
    case DistortionType::PERSPECTIVE:
        m_ptrSettings->setPerspectiveFovParams(pages_to_apply, m_pageParams.perspectiveParams().fovParams());
        break;
    case DistortionType::WARP:
        m_ptrSettings->setDewarpingFovParams(pages_to_apply, m_pageParams.dewarpingParams().fovParams());
        break;
    default:
        break;
    }

    emit invalidateAllThumbnails();
}

void
OptionsWidget::frameParamsAppliedTo(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    switch (m_pageParams.distortionType())
    {
    case DistortionType::PERSPECTIVE:
        m_ptrSettings->setPerspectiveFrameParams(pages, m_pageParams.perspectiveParams().frameParams());
        break;
    case DistortionType::WARP:
        m_ptrSettings->setDewarpingFrameParams(pages, m_pageParams.dewarpingParams().frameParams());
        break;
    default:
        break;
    }

    for (PageId const& page_id : pages)
    {
        emit invalidateThumbnail(page_id);
    }
}

void
OptionsWidget::frameParamsAppliedToAllPages(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    switch (m_pageParams.distortionType())
    {
    case DistortionType::PERSPECTIVE:
        m_ptrSettings->setPerspectiveFrameParams(pages, m_pageParams.perspectiveParams().frameParams());
        break;
    case DistortionType::WARP:
        m_ptrSettings->setDewarpingFrameParams(pages, m_pageParams.dewarpingParams().frameParams());
        break;
    default:
        break;
    }

    emit invalidateAllThumbnails();
}

void
OptionsWidget::bendParamsAppliedTo(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    std::set<PageId>  pages_to_apply;
    std::copy_if(
        pages.begin(),
        pages.end(),
        std::inserter(pages_to_apply, pages_to_apply.end()),
        [this](PageId const& page_id)
        {
            return this->m_pageId != page_id;
        }
    );

    m_ptrSettings->setDewarpingBendParams(pages_to_apply, m_pageParams.dewarpingParams().bendParams());

    for (PageId const& page_id : pages)
    {
        emit invalidateThumbnail(page_id);
    }
}

void
OptionsWidget::bendParamsAppliedToAllPages(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    std::set<PageId>  pages_to_apply;
    std::copy_if(
        pages.begin(),
        pages.end(),
        std::inserter(pages_to_apply, pages_to_apply.end()),
        [this](PageId const& page_id)
        {
            return this->m_pageId != page_id;
        }
    );

    m_ptrSettings->setDewarpingBendParams(pages_to_apply, m_pageParams.dewarpingParams().bendParams());

    emit invalidateAllThumbnails();
}

void
OptionsWidget::sizeParamsAppliedTo(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    switch (m_pageParams.distortionType())
    {
    case DistortionType::PERSPECTIVE:
        m_ptrSettings->setPerspectiveSizeParams(pages, m_pageParams.perspectiveParams().sizeParams());
        break;
    case DistortionType::WARP:
        m_ptrSettings->setDewarpingSizeParams(pages, m_pageParams.dewarpingParams().sizeParams());
        break;
    default:
        assert(!"Unreachable");
        break;
    }

    for (PageId const& page_id : pages)
    {
        emit invalidateThumbnail(page_id);
    }
}

void
OptionsWidget::sizeParamsAppliedToAllPages(std::set<PageId> const& pages)
{
    if (pages.empty())
    {
        return;
    }

    switch (m_pageParams.distortionType())
    {
    case DistortionType::PERSPECTIVE:
        m_ptrSettings->setPerspectiveSizeParams(pages, m_pageParams.perspectiveParams().sizeParams());
        break;
    case DistortionType::WARP:
        m_ptrSettings->setDewarpingSizeParams(pages, m_pageParams.dewarpingParams().sizeParams());
        break;
    default:
        assert(!"Unreachable");
        break;
    }

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

    updateAutoValuesOnPanels();
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
        preUpdateFovPanel(m_pageParams.perspectiveParams().fovParams());
        preUpdateFramePanel(m_pageParams.perspectiveParams().frameParams());
        preUpdateSizePanel(m_pageParams.perspectiveParams().sizeParams());
        break;
    case DistortionType::WARP:
        preUpdateFovPanel(m_pageParams.dewarpingParams().fovParams());
        preUpdateFramePanel(m_pageParams.dewarpingParams().frameParams());
        preUpdateBendPanel(m_pageParams.dewarpingParams().bendParams());
        preUpdateSizePanel(m_pageParams.dewarpingParams().sizeParams());
        break;
    default:
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
    ui.modeAutoBtn->setEnabled(true);
    ui.modeManualBtn->setEnabled(true);
    updateModeIndication(page_params.mode());

    switch (page_params.distortionType())
    {
    case DistortionType::PERSPECTIVE:
        postUpdateFovPanel(page_params.perspectiveParams().fovParams());
        postUpdateFramePanel(page_params.perspectiveParams().frameParams());
        postUpdateSizePanel(page_params.perspectiveParams().sizeParams());
        updateAutoValuesOnPanels();
        break;
    case DistortionType::WARP:
        postUpdateFovPanel(page_params.dewarpingParams().fovParams());
        postUpdateFramePanel(page_params.dewarpingParams().frameParams());
        postUpdateBendPanel(page_params.dewarpingParams().bendParams());
        postUpdateSizePanel(page_params.dewarpingParams().sizeParams());
        updateAutoValuesOnPanels();
        break;
    default:
        break;
    }

    enableDistortionDependentUiElements();

    if (page_params.distortionType() == DistortionType::ROTATION)
    {
        double const angle = page_params.rotationParams().compensationAngleDeg();
        setSpinBoxKnownState(degreesToSpinBox(angle));
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
OptionsWidget::modeChanged(bool const auto_mode)
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

    updateAutoValuesOnPanels();

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

    updateAutoValuesOnPanels();

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

    updateAutoValuesOnPanels();

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

    updateAutoValuesOnPanels();

    emit fovParamsSetByUser(fov_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::frameAutoManualModeChanged(bool auto_mode)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::FrameParams& frame_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().frameParams() :
        m_pageParams.dewarpingParams().frameParams();

    if (auto_mode)
    {
        frame_params.setMode(MODE_AUTO);
        ui.frameWidthSpinBox->setDisabled(true);
        ui.frameHeightSpinBox->setDisabled(true);
        ui.frameCenterXSpinBox->setDisabled(true);
        ui.frameCenterYSpinBox->setDisabled(true);
    }
    else
    {
        frame_params.setMode(MODE_MANUAL);
        ui.frameWidthSpinBox->setEnabled(true);
        ui.frameHeightSpinBox->setEnabled(true);
        ui.frameCenterXSpinBox->setEnabled(true);
        ui.frameCenterYSpinBox->setEnabled(true);
    }

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    updateAutoValuesOnPanels();

    if (auto_mode)
    {
        emit reloadRequested();
    }
}

void
OptionsWidget::frameWidthSpinBoxValueChanged(double width_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::FrameParams& frame_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().frameParams() :
        m_pageParams.dewarpingParams().frameParams();

    frame_params.setWidth(width_new);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    updateAutoValuesOnPanels();

    emit frameParamsSetByUser(frame_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::frameHeightSpinBoxValueChanged(double height_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::FrameParams& frame_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().frameParams() :
        m_pageParams.dewarpingParams().frameParams();

    frame_params.setHeight(height_new);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    updateAutoValuesOnPanels();

    emit frameParamsSetByUser(frame_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::frameCenterXSpinBoxValueChanged(double center_x_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::FrameParams& frame_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().frameParams() :
        m_pageParams.dewarpingParams().frameParams();

    frame_params.setCenterX(center_x_new);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    updateAutoValuesOnPanels();

    emit frameParamsSetByUser(frame_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::frameCenterYSpinBoxValueChanged(double center_y_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::FrameParams& frame_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().frameParams() :
        m_pageParams.dewarpingParams().frameParams();

    frame_params.setCenterY(center_y_new);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    updateAutoValuesOnPanels();

    emit frameParamsSetByUser(frame_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::bendAutoManualModeChanged(bool auto_mode)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::BendParams& bend_params = m_pageParams.dewarpingParams().bendParams();

    if (auto_mode)
    {
        bend_params.setMode(MODE_AUTO);
        ui.bendSlider->setDisabled(true);
        ui.bendSpinBox->setDisabled(true);
    }
    else
    {
        bend_params.setMode(MODE_MANUAL);
        ui.bendSlider->setEnabled(true);
        ui.bendSpinBox->setEnabled(true);
    }

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    updateAutoValuesOnPanels();

    if (auto_mode)
    {
        emit reloadRequested();
    }
}

void
OptionsWidget::bendSliderMoved(int value)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    double const bend = sliderToBend(value);

    dewarping::BendParams& bend_params = m_pageParams.dewarpingParams().bendParams();

    bend_params.setBend(bend);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    ui.bendSpinBox->setValue(bend);

    emit bendParamsSetByUser(bend_params);

    if (!ui.fovSlider->isSliderDown())
    {
        emit invalidateThumbnail(m_pageId);
    }
}

void
OptionsWidget::bendSliderReleased()
{
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::bendMinSpinBoxValueChanged(double bend_min_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::BendParams& bend_params = m_pageParams.dewarpingParams().bendParams();

    if (bend_params.bend() < bend_min_new)
    {
        bend_params.setBend(bend_min_new);
    }

    bend_params.setBendMin(bend_min_new);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    ui.bendSlider->setMinimum(bendToSlider(bend_min_new));
    ui.bendSpinBox->setMinimum(bend_min_new);
    ui.bendMaxSpinBox->setMinimum(bend_min_new);

    updateAutoValuesOnPanels();

    emit bendParamsSetByUser(bend_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::bendSpinBoxValueChanged(double bend_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::BendParams& bend_params = m_pageParams.dewarpingParams().bendParams();

    bend_params.setBend(bend_new);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    ui.bendSlider->setValue(bendToSlider(bend_new));

    updateAutoValuesOnPanels();

    emit bendParamsSetByUser(bend_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::bendMaxSpinBoxValueChanged(double bend_max_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::BendParams& bend_params = m_pageParams.dewarpingParams().bendParams();

    if (bend_params.bend() > bend_max_new)
    {
        bend_params.setBend(bend_max_new);
    }

    bend_params.setBendMax(bend_max_new);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    ui.bendSlider->setMaximum(bendToSlider(bend_max_new));
    ui.bendSpinBox->setMaximum(bend_max_new);
    ui.bendMinSpinBox->setMaximum(bend_max_new);

    updateAutoValuesOnPanels();

    emit bendParamsSetByUser(bend_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::sizeModeComboBoxIndexChanged(int idx)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::SizeParams& size_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().sizeParams() :
        m_pageParams.dewarpingParams().sizeParams();

    switch (idx)
    {
    case dewarping::SizeMode::BY_AREA:
        size_params.setMode(dewarping::SizeMode::BY_AREA);
        ui.sizeWidthSpinBox->setDisabled(true);
        ui.sizeHeightSpinBox->setDisabled(true);
        ui.sizeDistanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::FIT:
        size_params.setMode(dewarping::SizeMode::FIT);
        ui.sizeWidthSpinBox->setEnabled(true);
        ui.sizeHeightSpinBox->setEnabled(true);
        ui.sizeDistanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::STRETCH:
        size_params.setMode(dewarping::SizeMode::STRETCH);
        ui.sizeWidthSpinBox->setEnabled(true);
        ui.sizeHeightSpinBox->setEnabled(true);
        ui.sizeDistanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::BY_DISTANCE:
        size_params.setMode(dewarping::SizeMode::BY_DISTANCE);
        ui.sizeWidthSpinBox->setDisabled(true);
        ui.sizeHeightSpinBox->setDisabled(true);
        ui.sizeDistanceSpinBox->setEnabled(true);
        break;
    }

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    updateAutoValuesOnPanels();

    emit sizeParamsSetByUser(size_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::sizeWidthSpinBoxValueChanged(double width_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::SizeParams& size_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().sizeParams() :
        m_pageParams.dewarpingParams().sizeParams();

    size_params.setWidth(width_new);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    updateAutoValuesOnPanels();

    emit sizeParamsSetByUser(size_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::sizeHeightSpinBoxValueChanged(double height_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::SizeParams& size_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().sizeParams() :
        m_pageParams.dewarpingParams().sizeParams();

    size_params.setHeight(height_new);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    updateAutoValuesOnPanels();

    emit sizeParamsSetByUser(size_params);
    emit invalidateThumbnail(m_pageId);
}

void
OptionsWidget::sizeDistanceSpinBoxValueChanged(double distance_new)
{
    if (m_ignoreSignalsFromUiControls)
    {
        return;
    }

    dewarping::SizeParams& size_params =
        (m_pageParams.distortionType() == DistortionType::PERSPECTIVE) ?
        m_pageParams.perspectiveParams().sizeParams() :
        m_pageParams.dewarpingParams().sizeParams();

    size_params.setDistance(distance_new);

    m_ptrSettings->setPageParams(m_pageId, m_pageParams);

    updateAutoValuesOnPanels();

    emit sizeParamsSetByUser(size_params);
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
    ui.distortionModePanel->setDisabled(true);
    ui.rotationPanel->setDisabled(true);
    ui.fovPanel->setDisabled(true);
    ui.framePanel->setDisabled(true);
    ui.bendPanel->setDisabled(true);
    ui.sizePanel->setDisabled(true);
}

void
OptionsWidget::enableDistortionDependentUiElements()
{
    ui.distortionModePanel->setEnabled(true);
    ui.rotationPanel->setEnabled(true);
    ui.fovPanel->setEnabled(true);
    ui.framePanel->setEnabled(true);
    ui.bendPanel->setEnabled(true);
    ui.sizePanel->setEnabled(true);
}

void
OptionsWidget::setupUiForDistortionType(DistortionType::Type type)
{
    ScopedIncDec<int> guard(m_ignoreSignalsFromUiControls);

    m_distortionTypeButtons[type]->setChecked(true);

    ui.distortionModePanel->setVisible(type != DistortionType::NONE);
    ui.rotationPanel->setVisible(type == DistortionType::ROTATION);
    ui.fovPanel->setVisible(type == DistortionType::PERSPECTIVE || type == DistortionType::WARP);
    ui.framePanel->setVisible(type == DistortionType::PERSPECTIVE || type == DistortionType::WARP);
    ui.bendPanel->setVisible(type == DistortionType::WARP);
    ui.sizePanel->setVisible(type == DistortionType::PERSPECTIVE || type == DistortionType::WARP);
}

void
OptionsWidget::updateModeIndication(AutoManualMode const mode)
{
    ScopedIncDec<int> guard(m_ignoreSignalsFromUiControls);

    if (mode == MODE_AUTO)
    {
        ui.modeAutoBtn->setChecked(true);
    }
    else
    {
        ui.modeManualBtn->setChecked(true);
    }
}

void
OptionsWidget::preUpdateFovPanel(dewarping::FovParams const& fov_params)
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

    ui.fovMinSpinBox->setValue(ui.fovMinSpinBox->minimum());
    ui.fovSpinBox->setValue(ui.fovSpinBox->minimum());
    ui.fovMaxSpinBox->setValue(ui.fovMaxSpinBox->minimum());

    ui.fovMinSpinBox->setSpecialValueText(" ");
    ui.fovSpinBox->setSpecialValueText(" ");
    ui.fovMaxSpinBox->setSpecialValueText(" ");
}

void
OptionsWidget::postUpdateFovPanel(dewarping::FovParams const& fov_params)
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

    ui.fovMinSpinBox->setMaximum(fov_params.fovMax());
    ui.fovMaxSpinBox->setMinimum(fov_params.fovMin());
    ui.fovSpinBox->setRange(
        fov_params.fovMin(),
        fov_params.fovMax());

    ui.fovMinSpinBox->setValue(fov_params.fovMin());
    ui.fovSpinBox->setValue(fov_params.fov());
    ui.fovMaxSpinBox->setValue(fov_params.fovMax());

    ui.fovMinSpinBox->setSpecialValueText("");
    ui.fovSpinBox->setSpecialValueText("");
    ui.fovMaxSpinBox->setSpecialValueText("");
}

void
OptionsWidget::preUpdateFramePanel(dewarping::FrameParams const& frame_params)
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

    ui.frameWidthSpinBox->setValue(ui.frameWidthSpinBox->minimum());
    ui.frameHeightSpinBox->setValue(ui.frameHeightSpinBox->minimum());
    ui.frameCenterXSpinBox->setValue(ui.frameCenterXSpinBox->minimum());
    ui.frameCenterYSpinBox->setValue(ui.frameCenterYSpinBox->minimum());

    ui.frameWidthSpinBox->setSpecialValueText(" ");
    ui.frameHeightSpinBox->setSpecialValueText(" ");
    ui.frameCenterXSpinBox->setSpecialValueText(" ");
    ui.frameCenterYSpinBox->setSpecialValueText(" ");
}

void
OptionsWidget::postUpdateFramePanel(dewarping::FrameParams const& frame_params)
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

    ui.frameWidthSpinBox->setSpecialValueText("");
    ui.frameHeightSpinBox->setSpecialValueText("");
    ui.frameCenterXSpinBox->setSpecialValueText("");
    ui.frameCenterYSpinBox->setSpecialValueText("");
}

void
OptionsWidget::preUpdateBendPanel(dewarping::BendParams const& bend_params)
{
    if (bend_params.mode() == MODE_AUTO)
    {
        ui.bendAutoBtn->setChecked(true);
        ui.bendSlider->setDisabled(true);
        ui.bendSpinBox->setDisabled(true);
    }
    else
    {
        ui.bendManualBtn->setChecked(true);
        ui.bendSlider->setEnabled(true);
        ui.bendSpinBox->setEnabled(true);
    }

    ui.bendMinSpinBox->setValue(ui.bendMinSpinBox->minimum());
    ui.bendSpinBox->setValue(ui.bendSpinBox->minimum());
    ui.bendMaxSpinBox->setValue(ui.bendMaxSpinBox->minimum());

    ui.bendMinSpinBox->setSpecialValueText(" ");
    ui.bendSpinBox->setSpecialValueText(" ");
    ui.bendMaxSpinBox->setSpecialValueText(" ");
}

void
OptionsWidget::postUpdateBendPanel(dewarping::BendParams const& bend_params)
{
    if (bend_params.mode() == MODE_AUTO)
    {
        ui.bendAutoBtn->setChecked(true);
        ui.bendSlider->setDisabled(true);
        ui.bendSpinBox->setDisabled(true);
    }
    else
    {
        ui.bendManualBtn->setChecked(true);
        ui.bendSlider->setEnabled(true);
        ui.bendSpinBox->setEnabled(true);
    }

    ui.bendSlider->setRange(
        bendToSlider(bend_params.bendMin()),
        bendToSlider(bend_params.bendMax())
    );
    ui.bendSlider->setValue(bendToSlider(bend_params.bend()));

    ui.bendMinSpinBox->setMaximum(bend_params.bendMax());
    ui.bendMaxSpinBox->setMinimum(bend_params.bendMin());
    ui.bendSpinBox->setRange(
        bend_params.bendMin(),
        bend_params.bendMax()
    );

    ui.bendMinSpinBox->setValue(bend_params.bendMin());
    ui.bendSpinBox->setValue(bend_params.bend());
    ui.bendMaxSpinBox->setValue(bend_params.bendMax());

    ui.bendMinSpinBox->setSpecialValueText("");
    ui.bendSpinBox->setSpecialValueText("");
    ui.bendMaxSpinBox->setSpecialValueText("");
}

void
OptionsWidget::preUpdateSizePanel(dewarping::SizeParams const& size_params)
{
    switch (size_params.mode())
    {
    case dewarping::SizeMode::BY_AREA:
        ui.sizeWidthSpinBox->setDisabled(true);
        ui.sizeHeightSpinBox->setDisabled(true);
        ui.sizeDistanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::FIT:
        ui.sizeWidthSpinBox->setEnabled(true);
        ui.sizeHeightSpinBox->setEnabled(true);
        ui.sizeDistanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::STRETCH:
        ui.sizeWidthSpinBox->setEnabled(true);
        ui.sizeHeightSpinBox->setEnabled(true);
        ui.sizeDistanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::BY_DISTANCE:
        ui.sizeWidthSpinBox->setDisabled(true);
        ui.sizeHeightSpinBox->setDisabled(true);
        ui.sizeDistanceSpinBox->setEnabled(true);
        break;
    }

    ui.sizeModeComboBox->setCurrentIndex(-1);
    ui.sizeWidthSpinBox->setValue(ui.sizeWidthSpinBox->minimum());
    ui.sizeHeightSpinBox->setValue(ui.sizeHeightSpinBox->minimum());
    ui.sizeDistanceSpinBox->setValue(ui.sizeDistanceSpinBox->minimum());

    ui.sizeWidthSpinBox->setSpecialValueText(" ");
    ui.sizeHeightSpinBox->setSpecialValueText(" ");
    ui.sizeDistanceSpinBox->setSpecialValueText(" ");
}

void
OptionsWidget::postUpdateSizePanel(dewarping::SizeParams const& size_params)
{
    switch (size_params.mode())
    {
    case dewarping::SizeMode::BY_AREA:
        ui.sizeWidthSpinBox->setDisabled(true);
        ui.sizeHeightSpinBox->setDisabled(true);
        ui.sizeDistanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::FIT:
        ui.sizeWidthSpinBox->setEnabled(true);
        ui.sizeHeightSpinBox->setEnabled(true);
        ui.sizeDistanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::STRETCH:
        ui.sizeWidthSpinBox->setEnabled(true);
        ui.sizeHeightSpinBox->setEnabled(true);
        ui.sizeDistanceSpinBox->setDisabled(true);
        break;
    case dewarping::SizeMode::BY_DISTANCE:
        ui.sizeWidthSpinBox->setDisabled(true);
        ui.sizeHeightSpinBox->setDisabled(true);
        ui.sizeDistanceSpinBox->setEnabled(true);
        break;
    }

    ui.sizeModeComboBox->setCurrentIndex(size_params.mode());
    ui.sizeWidthSpinBox->setValue(size_params.width());
    ui.sizeHeightSpinBox->setValue(size_params.height());
    ui.sizeDistanceSpinBox->setValue(size_params.distance());

    ui.sizeWidthSpinBox->setSpecialValueText("");
    ui.sizeHeightSpinBox->setSpecialValueText("");
    ui.sizeDistanceSpinBox->setSpecialValueText("");
}

void
OptionsWidget::updateAutoValuesOnPanels()
try
{
    switch (m_pageParams.distortionType())
    {
    case DistortionType::PERSPECTIVE:
    {
        bool updateSize = true;
        bool updateFov = m_pageParams.perspectiveParams().fovParams().mode() == MODE_AUTO;

        if (updateSize || updateFov)
        {
            std::vector<QPointF> top_curve = {
                m_pageParams.perspectiveParams().corner(PerspectiveParams::TOP_LEFT),
                m_pageParams.perspectiveParams().corner(PerspectiveParams::TOP_RIGHT)
            };
            std::vector<QPointF> bottom_curve = {
                m_pageParams.perspectiveParams().corner(PerspectiveParams::BOTTOM_LEFT),
                m_pageParams.perspectiveParams().corner(PerspectiveParams::BOTTOM_RIGHT)
            };
            dewarping::CylindricalSurfaceDewarper dewarper(
                top_curve, bottom_curve,
                m_pageParams.perspectiveParams().fovParams(),
                m_pageParams.perspectiveParams().frameParams(),
                dewarping::BendParams(MODE_MANUAL, 0.0, 0.0, 0.0)
            );

            if (updateSize)
            {
                std::vector<QPointF> top_curve;
                std::vector<QPointF> bottom_curve;
                top_curve.push_back(m_pageParams.perspectiveParams().corner(PerspectiveParams::TOP_LEFT));
                top_curve.push_back(m_pageParams.perspectiveParams().corner(PerspectiveParams::TOP_RIGHT));
                bottom_curve.push_back(m_pageParams.perspectiveParams().corner(PerspectiveParams::BOTTOM_LEFT));
                bottom_curve.push_back(m_pageParams.perspectiveParams().corner(PerspectiveParams::BOTTOM_RIGHT));

                dewarping::ImageSize const image_size = dewarper.imageSize(
                    top_curve, bottom_curve, m_pageParams.perspectiveParams().sizeParams()
                );
                
                switch (ui.sizeModeComboBox->currentIndex())
                {
                case dewarping::SizeMode::BY_AREA:
                    ui.sizeWidthSpinBox->setValue(image_size.width);
                    ui.sizeHeightSpinBox->setValue(image_size.height);
                    ui.sizeDistanceSpinBox->setValue(image_size.distance);
                    break;
                case dewarping::SizeMode::FIT:
                    ui.sizeDistanceSpinBox->setValue(image_size.distance);
                    break;
                case dewarping::SizeMode::STRETCH:
                    ui.sizeDistanceSpinBox->setValue(image_size.distance);
                    break;
                case dewarping::SizeMode::BY_DISTANCE:
                    ui.sizeWidthSpinBox->setValue(image_size.width);
                    ui.sizeHeightSpinBox->setValue(image_size.height);
                    break;
                default:
                    assert(!"Unreachable");
                    break;
                }
            }

            if (updateFov)
            {
                ui.fovSpinBox->setValue(dewarper.fov());
                ui.fovSlider->setValue(fovToSlider(dewarper.fov()));
            }
        }
        break;
    }
    case DistortionType::WARP:
    {
        bool updateSize = true;
        bool updateFov = m_pageParams.dewarpingParams().fovParams().mode() == MODE_AUTO;
        bool updateBend = m_pageParams.dewarpingParams().bendParams().mode() == MODE_AUTO;

        if (updateSize || updateFov || updateBend)
        {
            dewarping::CylindricalSurfaceDewarper dewarper(
                m_pageParams.dewarpingParams().distortionModel().topCurve().polyline(),
                m_pageParams.dewarpingParams().distortionModel().bottomCurve().polyline(),
                m_pageParams.dewarpingParams().fovParams(),
                m_pageParams.dewarpingParams().frameParams(),
                m_pageParams.dewarpingParams().bendParams()
            );

            if (updateSize)
            {
                dewarping::ImageSize const image_size = dewarper.imageSize(
                    m_pageParams.dewarpingParams().distortionModel().topCurve().polyline(),
                    m_pageParams.dewarpingParams().distortionModel().bottomCurve().polyline(),
                    m_pageParams.dewarpingParams().sizeParams()
                );

                switch (ui.sizeModeComboBox->currentIndex())
                {
                case dewarping::SizeMode::BY_AREA:
                    ui.sizeWidthSpinBox->setValue(image_size.width);
                    ui.sizeHeightSpinBox->setValue(image_size.height);
                    ui.sizeDistanceSpinBox->setValue(image_size.distance);
                    break;
                case dewarping::SizeMode::FIT:
                    ui.sizeDistanceSpinBox->setValue(image_size.distance);
                    break;
                case dewarping::SizeMode::STRETCH:
                    ui.sizeDistanceSpinBox->setValue(image_size.distance);
                    break;
                case dewarping::SizeMode::BY_DISTANCE:
                    ui.sizeWidthSpinBox->setValue(image_size.width);
                    ui.sizeHeightSpinBox->setValue(image_size.height);
                    break;
                default:
                    assert(!"Unreachable");
                    break;
                }
            }

            if (updateFov)
            {
                ui.fovSpinBox->setValue(dewarper.fov());
                ui.fovSlider->setValue(fovToSlider(dewarper.fov()));
            }

            if (updateBend)
            {
                ui.bendSpinBox->setValue(dewarper.bend());
                ui.bendSlider->setValue(bendToSlider(dewarper.bend()));

            }
        }
        break;
    }
    default:
        break;
    }
}
catch (std::runtime_error const&)
{
    switch (m_pageParams.distortionType())
    {
    case DistortionType::PERSPECTIVE:
    {
        bool updateFov = m_pageParams.perspectiveParams().fovParams().mode() == MODE_AUTO;

        if (updateFov)
        {
            ui.fovSpinBox->setSpecialValueText("?");
            ui.fovSpinBox->setValue(ui.fovSpinBox->minimum());
        }

        break;
    }
    case DistortionType::WARP:
    {
        bool updateFov = m_pageParams.dewarpingParams().fovParams().mode() == MODE_AUTO;
        bool updateBend = m_pageParams.dewarpingParams().bendParams().mode() == MODE_AUTO;

        if (updateFov)
        {
            ui.fovSpinBox->setSpecialValueText("?");
            ui.fovSpinBox->setValue(ui.fovSpinBox->minimum());
        }

        if (updateBend)
        {
            ui.bendSpinBox->setSpecialValueText("?");
            ui.bendSpinBox->setValue(ui.bendSpinBox->minimum());
        }

        break;
    }
    default:
        break;
   }
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
