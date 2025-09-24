#pragma once

#include "fonts/JetBrainsMonoNerdFont-Regular.h"
#include "fonts/NotoSans-Regular.h"
#include <imgui.h>

namespace NebulaFontLoader {

static ImFont *Regular = nullptr;
static ImFont *Bold = nullptr;
static ImFont *Header = nullptr;

inline void loadFonts(float baseSize = 16.0f) {
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();

    // Ranges
    static const ImWchar nerd_ranges[] = {0xe000, 0xf8ff, 0};
    static const ImWchar star_ranges[] = {0x2218, 0x2606, 0x25E6, 0x2727, 0x2726, 0x25C8, 0};

    // Config for merging
    ImFontConfig mergeConfig;
    mergeConfig.MergeMode = true;
    mergeConfig.FontDataOwnedByAtlas = false; // Prevent ImGui from freeing static font data

    // 1) Create Regular font **with merged glyphs at creation**
    ImFontConfig regularConfig;
    regularConfig.FontDataOwnedByAtlas = false; // Prevent freeing
    Regular = io.Fonts->AddFontFromMemoryTTF(
        JetBrainsMonoNerdFont_Regular_ttf,
        JetBrainsMonoNerdFont_Regular_ttf_len,
        baseSize,
        &regularConfig,
        io.Fonts->GetGlyphRangesDefault()
    );

    // 2) Merge Nerd icons into Regular
    io.Fonts->AddFontFromMemoryTTF(
        JetBrainsMonoNerdFont_Regular_ttf,
        JetBrainsMonoNerdFont_Regular_ttf_len,
        baseSize,
        &mergeConfig,
        nerd_ranges
    );

    // 3) Merge NotoSans stars/bullets into Regular
    io.Fonts->AddFontFromMemoryTTF(
        NotoSans_Regular_ttf,
        NotoSans_Regular_ttf_len,
        baseSize,
        &mergeConfig,
        star_ranges
    );

    io.FontDefault = Regular;
}

inline ImFont *getRegularFont() { return Regular; }
inline ImFont *getBoldFont() { return Bold; }
inline ImFont *getHeaderFont() { return Header; }

} // namespace NebulaFontLoader
