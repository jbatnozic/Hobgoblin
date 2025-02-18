#pragma once

#include "Engine.hpp"

#include <Hobgoblin/Graphics.hpp>

#include <filesystem>

enum SpriteIds {
    // Kraken
    SPR_KRAKEN_BODY = 0,
    SPR_KRAKEN_FINS = 1,
    // Diver
    SPR_DIVER_ALIVE = 2,
    // Mountain
    SPR_TERRAIN = 3,
    // Miscellaneous
    SPR_POWER      = 4,
    SPR_BACKGROUND = 5,
    SPR_LAYOUT     = 6,
    SPR_SPONGE     = 7,
    SPR_PEARL      = 8,
    SPR_SHELL      = 9,
};

inline void LoadSprites(hg::gr::SpriteLoader& aSpriteLoader) {
    std::filesystem::path root = std::filesystem::current_path();
    for (int i = 0; i < 10; i += 1) {
        if (std::filesystem::exists(root / "Assets")) {
            break;
        }
        root = root.parent_path();
    }

    aSpriteLoader.loadSpriteManifest(root / "Assets/Sprites/Manifest.txt");
}
