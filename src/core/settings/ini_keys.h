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

#ifndef INI_KEYS_H
#define INI_KEYS_H

#include "config.h"
#include <QSettings> // propagate header

#include "../filters/output/DespeckleLevel.h"
#include "../../exporting/ExportModes.h"
#include <QSize>

/* App settings */

extern const char* _key_app_maximized;
extern const bool _key_app_maximized_def;
extern const char* _key_app_language;
extern const char* _key_app_style;
extern const char* _key_app_stylsheet_dir;
#ifdef _WIN32
extern const char* _key_app_style_def;
extern const char* _key_app_stylsheet_dir_def;
#elif defined(Q_OS_OSX)
extern const char* _key_app_style_def;
extern const char* _key_app_stylsheet_dir_def;
#else
extern const char* _key_app_style_def;
extern const char* _key_app_stylsheet_dir_def;
#endif
extern const char* _key_app_stylsheet_file;
extern const char* _key_app_empty_palette;
extern const bool _key_app_empty_palette_def;
extern const char* _key_app_open_filetype_filter;
extern const char* _key_app_docking_enabled;
extern const bool _key_app_docking_enabled_def;
#ifndef ENABLE_OPENJPEG
extern const char* _key_app_open_filetype_filter_def;
#else
extern const char* _key_app_open_filetype_filter_def;
#endif
extern const char* _key_app_alert_cmd;
#if (defined(Q_OS_UNIX) & (! defined(Q_OS_OSX)) & (!HAVE_CANBERRA))
extern const char* _key_app_alert_cmd_def;
#else
extern const char* _key_app_alert_cmd_def;
#endif

// following are not intended to be edited externally
extern const char* _key_app_settings_tree_state;
extern const char* _key_app_geometry;
extern const char* _key_app_state;

extern const char* _key_batch_dialog_start_from_current;
extern const bool _key_batch_dialog_start_from_current_def;
extern const char* _key_batch_dialog_remember_choice;
extern const bool _key_batch_dialog_remember_choice_def;
extern const char* _key_batch_processing_priority;

/* Thumbnails */

extern const char* _key_thumbnails_category;
extern const char* _key_thumbnails_max_cache_pixmap_size;
extern const QSize _key_thumbnails_max_cache_pixmap_size_def;
extern const char* _key_thumbnails_max_thumb_size;
extern const QSizeF _key_thumbnails_max_thumb_size_def;
extern const char* _key_thumbnails_non_focused_selection_highlight_color_adj;
extern const int _key_thumbnails_non_focused_selection_highlight_color_adj_def;
extern const char* _key_thumbnails_multiple_items_in_row;
extern const bool _key_thumbnails_multiple_items_in_row_def;
extern const char* _key_thumbnails_min_spacing;
extern const int _key_thumbnails_min_spacing_def;
extern const char* _key_thumbnails_boundary_adj_top;
extern const int _key_thumbnails_boundary_adj_top_def;
extern const char* _key_thumbnails_boundary_adj_bottom;
extern const int _key_thumbnails_boundary_adj_bottom_def;
extern const char* _key_thumbnails_boundary_adj_left;
extern const int _key_thumbnails_boundary_adj_left_def ;
extern const char* _key_thumbnails_boundary_adj_right;
extern const int _key_thumbnails_boundary_adj_right_def;
extern const char* _key_thumbnails_fixed_thumb_size;
extern const bool _key_thumbnails_fixed_thumb_size_def;
extern const char* _key_thumbnails_display_order_hints;
extern const bool _key_thumbnails_display_order_hints_def;
extern const char* _key_thumbnails_simulate_key_press_hint;
extern const bool _key_thumbnails_simulate_key_press_hint_def;

/* Export images dialog */

extern const char* _key_export_default_output_folder;
extern const char* _key_export_generate_blank_subscans;
extern const bool  _key_export_generate_blank_subscans_def;
extern const char* _key_export_use_sep_suffix;
extern const bool  _key_export_use_sep_suffix_def;
extern const char* _key_export_keep_original_color;
extern const bool  _key_export_keep_original_color_def;
extern const char* _key_export_to_multipage;
extern const bool  _key_export_to_multipage_def;
extern const char* _key_export_split_mixed_settings;
namespace exporting {
extern const int _key_export_split_mixed_settings_def;
}

/* insert image disalog */

extern const char* _key_dont_use_native_dialog;
extern const bool _key_dont_use_native_dialog_def;

/* Page split */

extern const char* _key_page_split_apply_cut_enabled;
extern const bool _key_page_split_apply_cut_enabled_def;
extern const char* _key_page_split_apply_cut_default;
extern const bool _key_page_split_apply_cut_default_def;

/* Content zone selection */

extern const char* _key_content_sel_STE_tweak;
extern const bool _key_content_sel_STE_tweak_def;
extern const char* _key_content_sel_page_detection_enabled;
extern const bool _key_content_sel_page_detection_enabled_def;
extern const char* _key_content_sel_page_detection_fine_tune_corners;
extern const bool _key_content_sel_page_detection_fine_tune_corners_def;
extern const char* _key_content_sel_page_detection_borders;
extern const bool _key_content_sel_page_detection_borders_def;
extern const char* _key_content_sel_content_color;
extern const char* _key_content_sel_content_color_def;

extern const char* _key_content_sel_page_detection_fine_tune_corners_is_on_by_def;
extern const bool _key_content_sel_page_detection_fine_tune_corners_is_on_by_def_def;
extern const char* _key_content_sel_page_detection_target_page_size_enabled;
extern const bool _key_content_sel_page_detection_target_page_size_enabled_def;
extern const char* _key_content_sel_page_detection_target_page_size;
extern const QSizeF _key_content_sel_page_detection_target_page_size_def;
extern const char* _key_content_sel_page_detection_borders_top;
extern const double _key_content_sel_page_detection_borders_top_def;
extern const char* _key_content_sel_page_detection_borders_left;
extern const double _key_content_sel_page_detection_borders_left_def;
extern const char* _key_content_sel_page_detection_borders_right;
extern const double _key_content_sel_page_detection_borders_right_def;
extern const char* _key_content_sel_page_detection_borders_bottom;
extern const double _key_content_sel_page_detection_borders_bottom_def;

/* Page Layout: Margins */

extern const char* _key_margins_default_top;
extern const double _key_margins_default_top_def;
extern const char* _key_margins_default_bottom;
extern const double _key_margins_default_bottom_def;
extern const char* _key_margins_default_left;
extern const double _key_margins_default_left_def;
extern const char* _key_margins_default_right;
extern const double _key_margins_default_right_def;
extern const char* _key_margins_default_units;
extern const int _key_margins_default_units_def;
extern const char* _key_margins_auto_margins_enabled;
extern const bool _key_margins_auto_margins_enabled_def;
extern const char* _key_margins_auto_margins_default;
extern const bool _key_margins_auto_margins_default_def;
extern const char* _key_margins_linked_hor;
extern const bool _key_margins_linked_hor_def;
extern const char* _key_margins_linked_ver;
extern const bool _key_margins_linked_ver_def;

/* Page Layout: Alignment */

extern const char* _key_alignment_original_enabled;
extern const bool  _key_alignment_original_enabled_def;
extern const char* _key_alignment_automagnet_enabled;
extern const bool  _key_alignment_automagnet_enabled_def;
extern const char* _key_alignment_default_alig_vert;
extern const char* _key_alignment_default_alig_hor;
extern const char* _key_alignment_default_alig_null;
extern const bool _key_alignment_default_alig_null_def;
extern const char* _key_alignment_default_alig_tolerance;
extern const double _key_alignment_default_alig_tolerance_def;

/* Output stage */

extern const char* _key_output_show_orig_on_space;
extern const bool _key_output_show_orig_on_space_def;
extern const char* _key_output_default_dpi_x;
extern const int _key_output_default_dpi_x_def;
extern const char* _key_output_default_dpi_y;
extern const int _key_output_default_dpi_y_def;
extern const char* _key_output_bin_threshold_min;
extern const int _key_output_bin_threshold_min_def;
extern const char* _key_output_bin_threshold_max;
extern const int _key_output_bin_threshold_max_def;
extern const char* _key_output_bin_threshold_default;
extern const int _key_output_bin_threshold_default_def;
extern const char* _key_output_despeckling_default_lvl;
extern const output::DespeckleLevel _key_output_despeckling_default_lvl_def;
extern const char* _key_output_foreground_layer_control_threshold;
extern const bool _key_output_foreground_layer_control_threshold_def;

extern const char* _key_output_foreground_layer_enabled;
extern const bool _key_output_foreground_layer_enabled_def;
extern const char* _key_output_picture_layer_enabled;
extern const bool _key_output_picture_layer_enabled_def;


extern const char* _key_output_metadata_copy_icc;
extern const bool _key_output_metadata_copy_icc_def;

/* Misc */

extern const char* _key_autosave_inputdir;
extern const char* _key_autosave_enabled;
extern const bool _key_autosave_enabled_def;
extern const char* _key_autosave_time_period_min;
extern const int _key_autosave_time_period_min_def;
extern const char* _key_debug_enabled;
extern const bool _key_debug_enabled_def;
extern const char* _key_dpi_predefined_list;
extern const char* _key_dpi_predefined_list_def;
extern const char* _key_dpi_change_list;
extern const char* _key_dpi_change_list_def;
extern const char* _key_project_last_dir;
extern const char* _key_hot_keys_jump_forward_pg_num;
extern const int _key_hot_keys_jump_forward_pg_num_def;
extern const char* _key_hot_keys_jump_backward_pg_num;
extern const char* _key_hot_keys_scheme_ver;
extern const char* _key_hot_keys_cnt;
extern const char* _key_hot_keys_group_id;
extern const int _key_hot_keys_jump_backward_pg_num_def;
extern const char* _key_project_last_input_dir;
extern const char* _key_recent_projects;
extern const char* _key_recent_projects_item;
extern const char* _key_use_3d_accel;
extern const bool _key_use_3d_accel_def;

extern const char* _key_deskew_controls_color;
extern const char* _key_deskew_controls_color_def;
extern const char* _key_deskew_deviant_enabled;
extern const bool _key_deskew_deviant_enabled_def;
extern const char* _key_dewarping_spline_points;
extern const int _key_dewarping_spline_points_def;
extern const char* _key_select_content_deviant_enabled;
extern const bool _key_select_content_deviant_enabled_def;
extern const char* _key_margins_deviant_enabled;
extern const bool _key_margins_deviant_enabled_def;

extern const char* _key_tiff_compr_method_bw;
extern const char* _key_tiff_compr_method_bw_def;
extern const char* _key_tiff_compr_method_color;
extern const char* _key_tiff_compr_method_color_def;
extern const char* _key_tiff_compr_horiz_pred;
extern const bool _key_tiff_compr_horiz_pred_def;
extern const char* _key_tiff_compr_show_all;
extern const bool _key_tiff_compr_show_all_def;

extern const char* _key_mode_bw_disable_smoothing;
extern const bool _key_mode_bw_disable_smoothing_def;
extern const char* _key_zone_editor_min_angle;
extern const float _key_zone_editor_min_angle_def;
extern const char* _key_picture_zones_layer_sensitivity;
extern const int _key_picture_zones_layer_sensitivity_def;
extern const char* _key_foreground_layer_adj_override;
extern const int _key_foreground_layer_adj_override_def;
extern const char* _key_mouse_ignore_system_wheel_settings;
extern const bool _key_mouse_ignore_system_wheel_settings_def;

#endif // INI_KEYS_H
