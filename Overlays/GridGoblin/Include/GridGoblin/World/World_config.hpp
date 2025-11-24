// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <GridGoblin/Model/Building_block.hpp>
#include <GridGoblin/Model/Chunk_extension.hpp>

#include <Hobgoblin/Common.hpp>

#include <filesystem>

namespace jbatnozic {
namespace gridgoblin {

namespace hg = ::jbatnozic::hobgoblin;

//! Configuration parameters pertaining to the contents of a World instance.
struct ContentsConfig {
    //! Total count of chunks in the horizontal (X) direction.
    //! Must be between 1 and 4096.
    hg::PZInteger chunkCountX;
    //! Total count of chunks in the vertical (Y) direction.
    //! Must be between 1 and 4096.
    hg::PZInteger chunkCountY;

    //! Total count of cells in a single chunk in the horizontal (X) direction.
    //! Must be between 1 and 1024.
    hg::PZInteger cellsPerChunkX;
    //! Total count of cells in a single chunk in the vertical (Y) direction.
    //! Must be between 1 and 1024.
    hg::PZInteger cellsPerChunkY;

    //! Building blocks of chunks that will be allocated and available for use.
    BuildingBlockMask buildingBlocks = BuildingBlockMask::ALL;

    //! The width and height of a single cell, in pixels. Must be positive.
    double cellResolution = 32.0;

    //! The implied height of all walls, in pixels. Note that this isn't the height of sprites, but
    //! the 'logical' height of walls. This value is needed for proper rendering.
    double wallHeight = 96.0;

    //! The 'openness' of a cell is defined as follows (openness = N, for brevity):
    //! - if `N == 0`, then the cell is solid.
    //!
    //! - if `N > 0 && IsOdd(N)`, then it is possible to place an object of size N x N or smaller
    //!   in this cell (such that their centers match) and it won't collide with any cells.
    //!
    //! - if `N > 0 && IsEven(N)`, then there exists at least one combination of numbers
    //!   xOff and yOff (xOff <= N/2, yOff <= N/2) such that it is possible to place an object
    //!   of size N x N or smaller in this cell, in a way that the center of the cell matches the
    //!   center of the object shifted by xOff along the X axis and by yOff along the Y axis.
    //!
    //! \note in the above definition, when we say that an object is of size N x N cells, that
    //!       really means that it's of size N*cellResolution x N*cellResolution (pixels). The
    //!       same applies for offsets xOff and yOff - they are smaller than N*cellResolution/2.
    //!
    //! \note cell openness is intended to be used for pathfinding algorithms: Paths of objects
    //!       that are up to 1 cell in width and height can pass through cells where openness is
    //!       1 or greater, objects that are up to 2 cells in width and height can pass through
    //!       cells whose openness is 2 or greater, and so on.
    //!
    //! This field, `maxCellOpenness`, lets the openness algorithm know at which point to stop.
    //! Determining whether openness is 0 or 1 needs checking only a single cell, but checking
    //! whether it's 2 usually needs checking 8 more cells, then checking whether it's 3
    //! needs checking 16 more cells and so on. Because this gets increasingly more expensive
    //! for larger values, make sure to set `maxCellOpenness` to no more than the largest object
    //! you expect to have in your game world.
    //!
    //! \warning `maxCellOpenness` must be either 0, or an odd number (because the first step of
    //! the openness algorithm checks whether openness is 0 or 1, then the second step checks
    //! whether it's 2 or 3, and so on, so it's not possible to stop at an even number - unless
    //! that number is zero, in which case there's really no check being performed at all).
    //! `maxCellOpenness` must also NOT be greater than either `cellsPerChunkX` or `cellsPerChunkY`,
    //! or 15 (which ever is the smallest).
    std::uint8_t maxCellOpenness = 0;

    //! The maximum number of chunks that can stay loaded and available after pruning a World instance
    //! even when they are not in any of the active areas. Note that an instance of each enabled
    //! building block (see `buildingBlocks` and the `cell` namespace) is allocated for each cell,
    //! and that that could be up to 24 bytes per cell, so take this into consideration when setting
    //! this value. The minimum is 0, though it is strongly not recommended.
    //!
    //! \warning openness values (see `maxCellOpenness`) can be inaccurate near chunks that aren't
    //!          currently loaded, though they are guaranteed to never be higher than the correct
    //!          values.
    //!
    //! \see World, ActiveArea
    hg::PZInteger maxLoadedNonessentialChunks = 0;

    //! Method to check if a configuration object is valid.
    //! \throws hg::InvalidArgumentError if the object is not valid.
    //! \returns the same configuration object that was passed in.
    static ContentsConfig& validate(ContentsConfig& aConfig);

    //! Method to check if a configuration object is valid.
    //! \throws hg::InvalidArgumentError if the object is not valid.
    //! \returns the same configuration object that was passed in.
    static const ContentsConfig& validate(const ContentsConfig& aConfig);

    //! Save the config object in a file.
    static void saveToFile(const ContentsConfig& aConfig, const std::filesystem::path& aFilePath);

    //! When initializing a GridGoblin World, it will expect to find a contents config file (with the
    //! same name as is stored in the constant `CONTENTS_CONFIG_FILE_NAME`) in its storage directory,
    //! and this is a helper function to save the config file in this directory without having to
    //! provide the file name explicitly.
    static void saveToStorageDirectory(const ContentsConfig&        aConfig,
                                       const std::filesystem::path& aStorageDirectory);

    //! Load the config object from a file.
    static ContentsConfig loadFromFile(const std::filesystem::path& aFilePath);

    //! \see saveToStorageDirectory
    static ContentsConfig loadFromStorageDirectory(const std::filesystem::path& aStorageDirectory);
};

//! Name of the file storing the contents config inside of a World Storage Directory.
constexpr const char* CONTENTS_CONFIG_FILE_NAME = "contents_config.txt";

//! Configuration parameters pertaining to persisting the World data on disk, and to loading it.
struct StorageConfig {
    //! Directory where the World's data is to be saved.
    //! \note multiple files and subfolders may be created at this location.
    std::filesystem::path storageDirectory = "";

    //! This flag controls the behaviour of the World constructor in the case when `storageDirectory`
    //! points to a directory that is empty and/or missing the crucial files that represent a World:
    //! - when `true`, the constructor will be allowed to initialize new (empty) World storage in this
    //!   directory.
    //! - when `false`, the constructor will throw an exception.
    bool allowCreateNew = true;

    //! This flag controls the behaviour of the World constructor in the case when `storageDirectory`
    //! points to a directory that already contains the crucial files that represent a World:
    //! - when `false`, the constructor will throw an exception unless the stored `ContentsConfig` is
    //!   exactly the same as the one passed to the World constructor.
    //! - when `true`, the stored `ContentsConfig` will be overwritten with the one passed to the
    //!   world constructor.
    //!
    //! \warning there are a few very important considerations when setting this to `true`:
    //!          - it's recommended to use `ContentsConfig::loadFromStorageDirectory` to load the config
    //!            manually and compare it to the one you want to use - and check for incompatibilities -
    //!            if you intend to overwrite.
    //!          - changing certain fields is not supported after a world is created, and you'll likely
    //!            encounter unrecoverable errors if you try to do so:
    //!             - `chunkCountX/chunkCountY`       - OK TO CHANGE*
    //!             - `cellsPerChunkX/cellsPerChunkX` - NOT SUPPORTED
    //!             - `buildingBlocks`                - NOT SUPPORTED
    //!             - `cellResolution`                - OK TO CHANGE
    //!             - `wallHeight`                    - OK TO CHANGE
    //!             - `maxCellOpenness`               - NOT SUPPORTED
    //!             - `maxLoadedNonessentialChunks`   - OK TO CHANGE
    //!
    //!            *- if you reduce the number of chunks in the world, the on-disk cache will not be
    //!               automatically cleaned up to remove the chunks which technically no longer belong.
    bool allowOverwriteConfig = false;
};

inline void ContentsConfig::saveToStorageDirectory(const ContentsConfig&        aConfig,
                                                   const std::filesystem::path& aStorageDirectory) {
    saveToFile(aConfig, aStorageDirectory / CONTENTS_CONFIG_FILE_NAME);
}

inline ContentsConfig ContentsConfig::loadFromStorageDirectory(
    const std::filesystem::path& aStorageDirectory) {
    return loadFromFile(aStorageDirectory / CONTENTS_CONFIG_FILE_NAME);
}

inline const ContentsConfig& ContentsConfig::validate(const ContentsConfig& aConfig) {
    return validate(const_cast<ContentsConfig&>(aConfig));
}

} // namespace gridgoblin
} // namespace jbatnozic
