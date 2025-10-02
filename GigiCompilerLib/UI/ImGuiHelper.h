///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

struct ImVec4;

//
// like ImGui::InputText() but work with std::string (currently limited size)
// and adds ".." button with file open dialog.
// Known issues:
// * Tooltip only shows up when hovering over the label
// * When the usable width get too small the button should disappear
//
// Usage:
//   if (ImGui_File("File", desc.texture.fileName))
//       setSomethingDirty();
//   ShowToolTip("My tooltip");
//
// @param label see ImGui::InputText()
// @param e.g. filterList e.g. "*;jpeg;jpg;png;bmp;hdr;psd;tga;gif;pic;pgm;ppm"
// @return see ImGui::InputText()
bool ImGui_File(const char* label, std::string& inOutName, const char* filterList);

// like ImGui::Checkbox(); but right aligned like other properties
bool ImGui_Checkbox(const char* label, bool *value);

// replacement for ImGui::Checkbox(label, value) but more compact UI and with color
bool ImGui_CheckboxButton(const char* label, bool* value, ImVec4 color);

// like MenuItem but prints the path differently from the filename for better user experience
// first path then file
// @param fileNameWithPath skipped if ""
bool ImGui_PathFileMenuItem(const char* fileNameWithPath, int index);

// like MenuItem but prints the path differently from the filename for better user experience
// first file, then path
bool ImGui_FilePathMenuItem(const char* fileNameWithPath, int index);

