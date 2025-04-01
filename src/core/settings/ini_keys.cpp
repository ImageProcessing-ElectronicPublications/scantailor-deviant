/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) Joseph Artsimovich <joseph_a@mail.ru>

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

#include "ini_keys.h"

/* App settings */

const char* _key_app_maximized = "main_window/maximized";
const bool _key_app_maximized_def = false;
const char* _key_app_language = "main_window/language";
const char* _key_app_style = "main_window/style";
const char* _key_app_stylsheet_dir = "main_window/qss_dir";
#ifdef _WIN32
const char* _key_app_style_def = "windows";
const char* _key_app_stylsheet_dir_def = STYLESHEETS_DIR_REL;
#elif defined(Q_OS_OSX)
const char* _key_app_style_def = "macintosh";
const char* _key_app_stylsheet_dir_def = STYLESHEETS_DIR_REL;
#else
const char* _key_app_style_def = "fusion";
const char* _key_app_stylsheet_dir_def = STYLESHEETS_DIR_ABS;
#endif
const char* _key_app_stylsheet_file = "main_window/qss_file";
const char* _key_app_empty_palette = "main_window/palette";
const bool _key_app_empty_palette_def = false;
const char* _key_app_open_filetype_filter = "main_window/filetype_filter";
const char* _key_app_docking_enabled = "docking_panels/enabled";
const bool _key_app_docking_enabled_def = false;
#ifndef ENABLE_OPENJPEG
const char* _key_app_open_filetype_filter_def = "*.png *.tiff *.tif *.jpeg *.jpg *.bmp";
#else
const char* _key_app_open_filetype_filter_def = "*.png *.tiff *.tif *.jpeg *.jpg *.bmp *.jp2";
#endif
const char* _key_app_alert_cmd = "main_window/external_alarm_cmd";
#if (defined(Q_OS_UNIX) & (! defined(Q_OS_OSX)) & (!HAVE_CANBERRA))
const char* _key_app_alert_cmd_def = "play /usr/share/sounds/freedesktop/stereo/bell.oga";
#else
const char* _key_app_alert_cmd_def = "";
#endif

// following are not intended to be edited externally
const char* _key_app_settings_tree_state = "main_window/settings_tree_state";
const char* _key_app_geometry = "main_window/non_maximized_geometry";
const char* _key_app_state = "main_window/state";

const char* _key_batch_dialog_start_from_current = "batch_dialog/start_from_current_page";
const bool _key_batch_dialog_start_from_current_def = true;
const char* _key_batch_dialog_remember_choice = "batch_dialog/remember_choice";
const bool _key_batch_dialog_remember_choice_def = false;
const char* _key_batch_processing_priority = "settings/batch_processing_priority";

/* Thumbnails */

const char* _key_thumbnails_category = "thumbnails/";
const char* _key_thumbnails_max_cache_pixmap_size = "thumbnails/max_cache_pixmap_size";
const QSize _key_thumbnails_max_cache_pixmap_size_def = QSize(200, 200);
const char* _key_thumbnails_max_thumb_size = "thumbnails/max_thumb_size";
const QSizeF _key_thumbnails_max_thumb_size_def = QSizeF(250., 160.);
const char* _key_thumbnails_non_focused_selection_highlight_color_adj = "thumbnails/non_focused_selection_highlight_color_adj";
const int _key_thumbnails_non_focused_selection_highlight_color_adj_def = 40;
const char* _key_thumbnails_multiple_items_in_row = "thumbnails/list_multiple_items_in_row";
const bool _key_thumbnails_multiple_items_in_row_def = true;
const char* _key_thumbnails_min_spacing = "thumbnails/min_spacing";
const int _key_thumbnails_min_spacing_def = 3;
const char* _key_thumbnails_boundary_adj_top = "thumbnails/boundary_adj_top";
const int _key_thumbnails_boundary_adj_top_def = 5;
const char* _key_thumbnails_boundary_adj_bottom = "thumbnails/boundary_adj_bottom";
const int _key_thumbnails_boundary_adj_bottom_def = 5;
const char* _key_thumbnails_boundary_adj_left = "thumbnails/boundary_adj_left";
const int _key_thumbnails_boundary_adj_left_def  = 5;
const char* _key_thumbnails_boundary_adj_right = "thumbnails/boundary_adj_right";
const int _key_thumbnails_boundary_adj_right_def = 3;
const char* _key_thumbnails_fixed_thumb_size = "thumbnails/fixed_thumb_size";
const bool _key_thumbnails_fixed_thumb_size_def = false;
const char* _key_thumbnails_display_order_hints = "thumbnails/display_order_hints";
const bool _key_thumbnails_display_order_hints_def = true;
const char* _key_thumbnails_simulate_key_press_hint = "thumbnails/selection_key_press_simulation_hint";
const bool _key_thumbnails_simulate_key_press_hint_def = true;

/* Export images dialog */

const char* _key_export_default_output_folder = "settings/default_output_folder";
const char* _key_export_generate_blank_subscans = "settings/generate_blank_back_subscans";
const bool  _key_export_generate_blank_subscans_def = false;
const char* _key_export_use_sep_suffix = "settings/use_sep_suffix_for_background_subscans";
const bool  _key_export_use_sep_suffix_def = true;
const char* _key_export_keep_original_color = "settings/keep_original_color_illum_fore_subscans";
const bool  _key_export_keep_original_color_def = false;
const char* _key_export_to_multipage = "settings/export_to_multipage";
const bool  _key_export_to_multipage_def = false;
const char* _key_export_split_mixed_settings = "settings/split_mixed_settings";
namespace exporting {
const int _key_export_split_mixed_settings_def = (int) ExportModes(ExportMode::Foreground | ExportMode::Background);
}

/* insert image disalog */

const char* _key_dont_use_native_dialog = "add_file_dlg/dont_use_native_dialog";
const bool _key_dont_use_native_dialog_def = true;

/* Page split */

const char* _key_page_split_apply_cut_enabled = "apply_cut/enabled";
const bool _key_page_split_apply_cut_enabled_def = true;
const char* _key_page_split_apply_cut_default = "apply_cut/default";
const bool _key_page_split_apply_cut_default_def = false;

/* Content zone selection */

const char* _key_content_sel_STE_tweak = "content_selection/force_STE_reload_tweak";
const bool _key_content_sel_STE_tweak_def = false;
const char* _key_content_sel_page_detection_enabled = "page_detection/enabled";
const bool _key_content_sel_page_detection_enabled_def = false;
const char* _key_content_sel_page_detection_fine_tune_corners = "page_detection/fine_tune_page_corners";
const bool _key_content_sel_page_detection_fine_tune_corners_def = false;
const char* _key_content_sel_page_detection_borders = "page_detection/borders";
const bool _key_content_sel_page_detection_borders_def = false;
const char* _key_content_sel_content_color = "content_selection/content_color";
const char* _key_content_sel_content_color_def = "#320000ff";

const char* _key_content_sel_page_detection_fine_tune_corners_is_on_by_def = "page_detection/fine_tune_page_corners/default";
const bool _key_content_sel_page_detection_fine_tune_corners_is_on_by_def_def = false;
const char* _key_content_sel_page_detection_target_page_size_enabled = "page_detection/target_page_size/enabled";
const bool _key_content_sel_page_detection_target_page_size_enabled_def = false;
const char* _key_content_sel_page_detection_target_page_size = "page_detection/target_page_size";
const QSizeF _key_content_sel_page_detection_target_page_size_def = QSizeF(210., 297.);
const char* _key_content_sel_page_detection_borders_top = "page_detection/borders/top";
const double _key_content_sel_page_detection_borders_top_def = 0.;
const char* _key_content_sel_page_detection_borders_left = "page_detection/borders/left";
const double _key_content_sel_page_detection_borders_left_def = 0.;
const char* _key_content_sel_page_detection_borders_right = "page_detection/borders/right";
const double _key_content_sel_page_detection_borders_right_def = 0.;
const char* _key_content_sel_page_detection_borders_bottom = "page_detection/borders/bottom";
const double _key_content_sel_page_detection_borders_bottom_def = 0.;

/* Page Layout: Margins */

const char* _key_margins_default_top = "margins/default_top";
const double _key_margins_default_top_def = 0.;
const char* _key_margins_default_bottom = "margins/default_bottom";
const double _key_margins_default_bottom_def = 0.;
const char* _key_margins_default_left = "margins/default_left";
const double _key_margins_default_left_def = 0.;
const char* _key_margins_default_right = "margins/default_right";
const double _key_margins_default_right_def = 0.;
const char* _key_margins_default_units = "margins/default_units";
const int _key_margins_default_units_def = 0;
const char* _key_margins_auto_margins_enabled = "margins/auto_margins_enabled";
const bool _key_margins_auto_margins_enabled_def = false;
const char* _key_margins_auto_margins_default = "margins/default_auto_margins";
const bool _key_margins_auto_margins_default_def = false;
const char* _key_margins_linked_hor = "margins/leftRightLinked";
const bool _key_margins_linked_hor_def = true;
const char* _key_margins_linked_ver = "margins/topBottomLinked";
const bool _key_margins_linked_ver_def = true;

/* Page Layout: Alignment */

const char* _key_alignment_original_enabled =  "alignment/original_enabled";
const bool  _key_alignment_original_enabled_def = false;
const char* _key_alignment_automagnet_enabled =  "alignment/automagnet_enabled";
const bool  _key_alignment_automagnet_enabled_def = false;
const char* _key_alignment_default_alig_vert = "alignment/default_alignment_vert";
const char* _key_alignment_default_alig_hor = "alignment/default_alignment_hor";
const char* _key_alignment_default_alig_null = "alignment/default_alignment_null";
const bool _key_alignment_default_alig_null_def = false;
const char* _key_alignment_default_alig_tolerance = "alignment/default_alignment_tolerance";
const double _key_alignment_default_alig_tolerance_def = 0.2;

/* Output stage */

const char* _key_output_show_orig_on_space = "output/display_orig_page_on_key_press";
const bool _key_output_show_orig_on_space_def = false;
const char* _key_output_default_dpi_x = "output/default_dpi_x";
const int _key_output_default_dpi_x_def = 600;
const char* _key_output_default_dpi_y = "output/default_dpi_y";
const int _key_output_default_dpi_y_def = 600;
const char* _key_output_bin_threshold_min = "output/binrization_threshold_control_min";
const int _key_output_bin_threshold_min_def = -50;
const char* _key_output_bin_threshold_max = "output/binrization_threshold_control_max";
const int _key_output_bin_threshold_max_def = 50;
const char* _key_output_bin_threshold_default = "output/binrization_threshold_control_default";
const int _key_output_bin_threshold_default_def = 0;
const char* _key_output_despeckling_default_lvl = "despeckling/default_level";
const output::DespeckleLevel _key_output_despeckling_default_lvl_def = output::DespeckleLevel::DESPECKLE_CAUTIOUS;
const char* _key_output_foreground_layer_control_threshold = "foreground_layer/control_threshold";
const bool _key_output_foreground_layer_control_threshold_def = false;

const char* _key_output_foreground_layer_enabled = "foreground_layer/enabled";
const bool _key_output_foreground_layer_enabled_def = true;
const char* _key_output_picture_layer_enabled = "picture_zones_layer/enabled";
const bool _key_output_picture_layer_enabled_def = true;


const char* _key_output_metadata_copy_icc = "metadata/copy_icc";
const bool _key_output_metadata_copy_icc_def = false;

/* Misc */

const char* _key_autosave_inputdir = "auto-save_project/_inputDir";
const char* _key_autosave_enabled = "auto-save_project/enabled";
const bool _key_autosave_enabled_def = false;
const char* _key_autosave_time_period_min = "auto-save_project/time_period_min";
const int _key_autosave_time_period_min_def = 5;
const char* _key_debug_enabled = "debug_mode/enabled";
const bool _key_debug_enabled_def = false;
const char* _key_dpi_predefined_list = "dpi/predefined_list";
const char* _key_dpi_predefined_list_def = "300x300,400x400,600x600,1200x1200";
const char* _key_dpi_change_list = "dpi/change_dpi_list";
const char* _key_dpi_change_list_def = "300,400,600,1200" ;
const char* _key_project_last_dir = "project/lastDir";
const char* _key_hot_keys_jump_forward_pg_num = "hot_keys/jump_forward_pg_num";
const int _key_hot_keys_jump_forward_pg_num_def = 5;
const char* _key_hot_keys_jump_backward_pg_num = "hot_keys/jump_backward_pg_num";
const char* _key_hot_keys_scheme_ver = "hot_keys/scheme_ver";
const char* _key_hot_keys_cnt = "hot_keys/count";
const char* _key_hot_keys_group_id = "hot_keys/group_%1";
const int _key_hot_keys_jump_backward_pg_num_def = 5;
const char* _key_project_last_input_dir = "lastInputDir";
const char* _key_recent_projects = "project/recent";
const char* _key_recent_projects_item = "path";
const char* _key_use_3d_accel = "settings/use_3d_acceleration";
const bool _key_use_3d_accel_def = false;

const char* _key_deskew_controls_color = "deskew/controls_color";
const char* _key_deskew_controls_color_def = "#5A0000ff";
const char* _key_deskew_deviant_enabled = "deskew_deviant/enabled";
const bool _key_deskew_deviant_enabled_def = false;
extern const char* _key_dewarping_spline_points = "dewarping/spline_points";
extern const int _key_dewarping_spline_points_def = 5;
const char* _key_select_content_deviant_enabled = "select_content_deviant/enabled";
const bool _key_select_content_deviant_enabled_def = false;
const char* _key_margins_deviant_enabled = "margins_deviant/enabled";
const bool _key_margins_deviant_enabled_def = false;

const char* _key_tiff_compr_method_bw = "tiff_compression/method";
const char* _key_tiff_compr_method_bw_def = "LZW";
const char* _key_tiff_compr_method_color = "tiff_compression/method_color";
const char* _key_tiff_compr_method_color_def = "LZW";
const char* _key_tiff_compr_horiz_pred = "tiff_compression/use_horizontal_predictor";
const bool _key_tiff_compr_horiz_pred_def = false;
const char* _key_tiff_compr_show_all = "tiff_compression/show_all";
const bool _key_tiff_compr_show_all_def = false;

const char* _key_mode_bw_disable_smoothing = "mode_bw/disable_smoothing";
const bool _key_mode_bw_disable_smoothing_def = false;
const char* _key_zone_editor_min_angle = "zone_editor/min_angle";
const float _key_zone_editor_min_angle_def = 3.;
const char* _key_picture_zones_layer_sensitivity = "picture_zones_layer/sensitivity";
const int _key_picture_zones_layer_sensitivity_def = 100;
const char* _key_foreground_layer_adj_override = "foreground_layer_adj_override";
const int _key_foreground_layer_adj_override_def = 0;
const char* _key_mouse_ignore_system_wheel_settings = "mouse/ignore_system_wheel_settings";
const bool _key_mouse_ignore_system_wheel_settings_def = true;
