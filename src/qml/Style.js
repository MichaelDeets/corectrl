// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright 2019 Juan Palacios <jpalaciosdev@gmail.com>

.pragma library;

var g_padding = 12;
var g_hover = true;
var g_tweakScale = 0.78;

var g_text = {
  size: 9,
  icon_size: 12,
  icon_size_tabbar: 16,
  family: "Sans"
};

var g_icon = {
  size: 32,
  small_size: 18,
  source_size: 64,

  // unicode button icons
  MENU:    "\u22EE", // ⋮
  MORE:    "\u22EF", // ⋯
  BACK:    "\u2190", // ←
  ADD:     "+",
  SPLIT:   "|",
};

var Theme = {
  accent: "orangered",
  accent_alt: "#909090",
  foreground: "white",
  background: "#303030",
};

var Menu = {
  background: "#424242"
};

var ToolBar = {
  text_color_msg: "#DADADA",
  text_color: "white",
  text_color_alt: "#909090"
};

var Split = {
  background: "#353535",
  thickness: 6,

  Handle: {
    background: "#797979",
    thickness: 2,
    length: 9
  }
};

var TabBar = {
  Button: {
    accent_size: 2,
    background: "#383838",
    background_alt: "#484848",

    MainFooter: {
      background: "#555555",
      background_alt: "#6A6A6A"
    }
  }
};

var TextField = {
  padding: 8,
};

var Controls = {
  items_spacing: 4
};

var ModeSelector = {
  Header: {
    background: "#404040",
    padding: 8,
  },
  Body: {
    background: "#393939"
  }
};

var Graph = {
  background: "#20FFFFFF",
  ctl_background: "#AA202020"
};

var Profiles = {
  opacity_anim_duration: 200
};

var ToolTip = {
  delay: 1000,
  timeout: 3500
};

var CurveControl = {
  curve_amd_start_color: "lightskyblue",
  curve_color: "#ABFFFFFF",

  curve_opacity: 1.0,
  curve_opacity_alt: 0.3,

  axis_title_color: "#F0FFFFFF",
  axis_title_color_alt: "#80FFFFFF",

  axis_label_format: "%i",

  axis_label_color: "#C7FFFFFF",
  axis_label_color_alt: "#77FFFFFF",

  axis_color: "#E0FFFFFF",
  axis_color_alt: "#20FFFFFF",

  axis_grid_color: "#80FFFFFF",
  axis_grid_color_alt: "#20FFFFFF",

  axis_grid_minor_color: "#30FFFFFF",
  axis_grid_minor_color_alt: "#09FFFFFF",
};

var FVControl = {
  border_color: "#40666666",
  inner_padding: 18,
};

var Dialog = {
  list_color: "#FF343434"
};

var GroupBox = {
  text_size: 11,
  text_bold: true,
  background: "transparent",
  border_color: "#FF747474",
  border_color_alt: "#40747474",
  radius: 2
};

var RectItem = {
  background: "#484848",
  background_alt: "#404040",
  background_hover: "#505050",

  text_color: "#FAFAFA",
  text_color_alt: "#909090",

  padding: 10
};

var RectItemList = {
  spacing: 2
};
