// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#pragma once

#include <Engine.hpp>

#include <Hobgoblin/Alvin.hpp>
#include <Hobgoblin/ChipmunkPhysics.hpp>

namespace cinnabar {
namespace ovwcol {

enum EntityIds {
    EID_JUNK,
    // EID_WALL,
    // EID_SENSOR
};

enum EntityCategories {
    CAT_JUNK = 0x01,
    // CAT_WALL   = 0x02,
    // CAT_SENSOR = 0x04
};

class JunkEntity : public hg::alvin::EntityBase {
public:
    using EntitySuperclass = hg::alvin::EntityBase;

    static constexpr hg::alvin::EntityTypeId ENTITY_TYPE_ID = EID_JUNK;

    static constexpr cpBitmask ENTITY_DEFAULT_CATEGORY = CAT_JUNK;
    static constexpr cpBitmask ENTITY_DEFAULT_MASK     = CP_ALL_CATEGORIES;
};

#if 0
class WallInterface : public hg::alvin::EntityBase {
public:
    using EntitySuperclass = hg::alvin::EntityBase;

    static constexpr hg::alvin::EntityTypeId ENTITY_TYPE_ID = EID_WALL;

    static constexpr cpBitmask ENTITY_DEFAULT_CATEGORY = EID_WALL;
    static constexpr cpBitmask ENTITY_DEFAULT_MASK     = CAT_BALL;
};

class SensorInterface : public hg::alvin::EntityBase {
public:
    using EntitySuperclass = hg::alvin::EntityBase;

    static constexpr hg::alvin::EntityTypeId ENTITY_TYPE_ID = EID_SENSOR;

    static constexpr cpBitmask ENTITY_DEFAULT_CATEGORY = CAT_SENSOR;
    static constexpr cpBitmask ENTITY_DEFAULT_MASK     = CP_ALL_CATEGORIES;
};
#endif

} // namespace ovwcol

inline void InitOverworldCollisions(hg::alvin::MainCollisionDispatcher& aDispatcher,
                                    hg::NeverNull<cpSpace*>             aSpace) {
    aDispatcher.registerEntityType<ovwcol::JunkEntity>();
    aDispatcher.bind(aSpace);
}

} // namespace cinnabar
