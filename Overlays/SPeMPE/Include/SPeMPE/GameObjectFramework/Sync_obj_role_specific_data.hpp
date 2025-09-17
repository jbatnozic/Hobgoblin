// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef SPEMPE_GAME_OBJECT_FRAMEWORK_SYNC_OBJ_ROLE_SPECIFIC_DATA_HPP
#define SPEMPE_GAME_OBJECT_FRAMEWORK_SYNC_OBJ_ROLE_SPECIFIC_DATA_HPP

#include <algorithm>
#include <memory>
#include <new>
#include <type_traits>

namespace jbatnozic {
namespace spempe {
namespace detail {

// MARK: Inline data

// Primary template (assumes none of the template parameters are `void`).
template <class taMasterSpecificInlineData, class taDummySpecificInlineData>
class RoleSpecificInlineData {
private:
    static constexpr std::size_t _getStorageSize() {
        return std::max(sizeof(taDummySpecificInlineData), sizeof(taMasterSpecificInlineData));
    }

    static constexpr std::size_t _getStorageAlignment() {
        return std::max(alignof(taDummySpecificInlineData), alignof(taMasterSpecificInlineData));
    }

    alignas(_getStorageAlignment()) char _storage[_getStorageSize()];

protected:
    taMasterSpecificInlineData* _masterData = nullptr;
    taDummySpecificInlineData*  _dummyData  = nullptr;

    void __spempeimpl_initMasterSpecificInlineData() {
        _masterData = new (&_storage) taMasterSpecificInlineData;
    }

    void __spempeimpl_initDummySpecificInlineData() {
        _dummyData = new (&_storage) taDummySpecificInlineData;
    }

    void __spempeimpl_cleanupMasterSpecificInlineData() {
        if (_masterData) {
            _masterData->~taMasterSpecificInlineData();
            _masterData = nullptr;
        }
    }

    void __spempeimpl_cleanupDummySpecificInlineData() {
        if (_dummyData) {
            _dummyData->~taDummySpecificInlineData();
            _dummyData = nullptr;
        }
    }
};

// Partial specialization for when dummy data is `void`.
template <class taMasterSpecificInlineData>
class RoleSpecificInlineData<taMasterSpecificInlineData, void> {
private:
    taMasterSpecificInlineData _data;

protected:
    taMasterSpecificInlineData* _masterData = nullptr;

    void __spempeimpl_initMasterSpecificInlineData() {
        _masterData = &_data;
    }

    void __spempeimpl_initDummySpecificInlineData() {}

    void __spempeimpl_cleanupMasterSpecificInlineData() {}

    void __spempeimpl_cleanupDummySpecificInlineData() {}
};

// Partial specialization for when master data is `void`.
template <class taDummySpecificInlineData>
class RoleSpecificInlineData<void, taDummySpecificInlineData> {
private:
    taDummySpecificInlineData _data;

protected:
    taDummySpecificInlineData* _dummyData = nullptr;

    void __spempeimpl_initMasterSpecificInlineData() {}

    void __spempeimpl_initDummySpecificInlineData() {
        _dummyData = &_data;
    }

    void __spempeimpl_cleanupMasterSpecificInlineData() {}

    void __spempeimpl_cleanupDummySpecificInlineData() {}
};

// Full specialization for when all data is `void`.
template <>
class RoleSpecificInlineData<void, void> {
protected:
    void __spempeimpl_initMasterSpecificInlineData() {}

    void __spempeimpl_initDummySpecificInlineData() {}

    void __spempeimpl_cleanupMasterSpecificInlineData() {}

    void __spempeimpl_cleanupDummySpecificInlineData() {}
};

// MARK: Heap data

template <class T>
class MasterSpecificHeapData {
protected:
    std::unique_ptr<T> _masterData;

    void __spempeimpl_initMasterSpecificHeapData() {
        _masterData = std::make_unique<T>();
    }

    void __spempeimpl_cleanupMasterSpecificHeapData() {
        _masterData.reset();
    }
};

template <>
class MasterSpecificHeapData<void> {
protected:
    void __spempeimpl_initMasterSpecificHeapData() {}

    void __spempeimpl_cleanupMasterSpecificHeapData() {}
};

template <class T>
class DummySpecificHeapData {
protected:
    std::unique_ptr<T> _dummyData;

    void __spempeimpl_initDummySpecificHeapData() {
        _dummyData = std::make_unique<T>();
    }

    void __spempeimpl_cleanupDummySpecificHeapData() {
        _dummyData.reset();
    }
};

template <>
class DummySpecificHeapData<void> {
protected:
    void __spempeimpl_initDummySpecificHeapData() {}

    void __spempeimpl_cleanupDummySpecificHeapData() {}
};

// MARK: Main class

#ifdef _MSC_VER
// https://stackoverflow.com/a/55530422
#define EMPTY_BASES __declspec(empty_bases)
#else
#define EMPTY_BASES
#endif

template <class taMasterSpecificInlineData = void,
          class taDummySpecificInlineData  = void,
          class taMasterSpecificHeapData   = void,
          class taDummySpecificHeapData    = void>
class EMPTY_BASES RoleSpecificData
    : public RoleSpecificInlineData<taMasterSpecificInlineData, taDummySpecificInlineData>
    , public MasterSpecificHeapData<taMasterSpecificHeapData>
    , public DummySpecificHeapData<taDummySpecificHeapData> {
public:
    // Can't have both master data types as non-`void`.
    static_assert(std::is_same_v<taMasterSpecificInlineData, void> ||
                  std::is_same_v<taMasterSpecificHeapData, void>);

    // Can't have both dummy data types as non-`void`.
    static_assert(std::is_same_v<taDummySpecificInlineData, void> ||
                  std::is_same_v<taDummySpecificHeapData, void>);
};

#undef EMPTY_BASES

} // namespace detail

#define SPEMPE_RSDATA_INLINE(_master_specific_data_type_, _dummy_specific_data_type_) \
    _master_specific_data_type_, _dummy_specific_data_type_, void, void

#define SPEMPE_RSDATA_HEAP(_master_specific_data_type_, _dummy_specific_data_type_) \
    void, void, _master_specific_data_type_, _dummy_specific_data_type_

} // namespace spempe
} // namespace jbatnozic

#endif // !SPEMPE_GAME_OBJECT_FRAMEWORK_SYNC_OBJ_ROLE_SPECIFIC_DATA_HPP
