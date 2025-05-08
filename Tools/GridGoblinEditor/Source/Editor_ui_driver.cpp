// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Editor_ui_driver.hpp"

#include "Definitions.hpp"

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/Unicode.hpp>
#include <Hobgoblin/Utility/No_copy_no_move.hpp>
#include <RmlUi/Core.h>

namespace jbatnozic {
namespace gridgoblin {
namespace editor {

namespace {
constexpr auto LOG_ID = "GridGoblin.Editor";

class RmlUiDataModelError : public hg::TracedLogicError {
public:
    using hg::TracedLogicError::TracedLogicError;
};

#define RMLUI_DATAMODEL_ASSERT(_expr_)                                                   \
    do {                                                                                 \
        if (!(_expr_)) {                                                                 \
            HG_THROW_TRACED(RmlUiDataModelError, 0, "Expression '{}' failed.", #_expr_); \
        }                                                                                \
    } while (false)

template <class T>
void RegisterModel(Rml::DataModelConstructor& /*aDataModelCtor*/) {
    HG_UNREACHABLE("Non-specialized RegisterModel() should never be called.");
}

struct CellDefinitionModel {
    Rml::String definitionName;

    bool operator<(const CellDefinitionModel& aOther) const {
        return definitionName < aOther.definitionName;
    }
};

template <>
void RegisterModel<CellDefinitionModel>(Rml::DataModelConstructor& aDataModelCtor) {
    std::optional<Rml::StructHandle<CellDefinitionModel>> handle;
    RMLUI_DATAMODEL_ASSERT(handle = aDataModelCtor.RegisterStruct<CellDefinitionModel>());

    RMLUI_DATAMODEL_ASSERT(
        handle->RegisterMember("definitionName", &CellDefinitionModel::definitionName));
}

struct EditorUiModel {
    std::vector<CellDefinitionModel> cellDefinitions;
    std::size_t                      selectedIdx = 0;
};

template <>
void RegisterModel<EditorUiModel>(Rml::DataModelConstructor& aDataModelCtor) {
    std::optional<Rml::StructHandle<EditorUiModel>> handle;
    RMLUI_DATAMODEL_ASSERT(handle = aDataModelCtor.RegisterStruct<EditorUiModel>());

    RMLUI_DATAMODEL_ASSERT(aDataModelCtor.RegisterArray<std::vector<CellDefinitionModel>>());
    RMLUI_DATAMODEL_ASSERT(handle->RegisterMember("cellDefinitions", &EditorUiModel::cellDefinitions));
    RMLUI_DATAMODEL_ASSERT(handle->RegisterMember("selectedIdx", &EditorUiModel::selectedIdx));
}
} // namespace

///////////////////////////////////////////////////////////////////////////
// MARK: IMPL                                                            //
///////////////////////////////////////////////////////////////////////////

using MWindow = spempe::WindowManagerInterface;

class EditorUiDriver::Impl
    : hg::util::NonCopyable
    , hg::util::NonMoveable {
private:
    static constexpr auto DATA_MODEL_NAME = "editor-ui-driver-model";

public:
    explicit Impl(EditorUiDriver& aEditorUiDriver)
        : _super{aEditorUiDriver} {}

    ~Impl() {
        auto& guiContext = ccomp<MWindow>().getGUIContext();
        if (_document) {
            guiContext.UnloadDocument(_document);
            _document = nullptr;
        }
        guiContext.RemoveDataModel(DATA_MODEL_NAME);
    }

    template <class taComponent>
    taComponent& ccomp() const {
        return _super.ccomp<taComponent>();
    }

    void init(const std::filesystem::path& aAssetsDir, const std::filesystem::path& aDefinitionsDir) {
        SPEMPE_VALIDATE_GAME_CONTEXT_FLAGS(_super.ctx(), headless == false);

        _allDefinitions = _loadAllDefinitions(aDefinitionsDir);
        _editorUiModel  = _allDefinitionsToModel(_allDefinitions);

        auto& guiContext = ccomp<MWindow>().getGUIContext();
        _dataModelHandle = _setUpDataBinding(guiContext);

        hg::rml::PreprocessRcssFile((aAssetsDir / "editor_ui.rcss.fp").string());
        _document = guiContext.LoadDocument((aAssetsDir / "editor_ui.rml").string());
        if (_document) {
            _document->Show();
            _documentVisible = true;
            HG_LOG_INFO(LOG_ID, "RMLUI Document loaded successfully.");
        } else {
            HG_LOG_ERROR(LOG_ID, "RMLUI Document could not be loaded.");
        }
    }

    void eventBeginUpdate() {}

    void eventUpdate1() {
        auto& winMgr = ccomp<MWindow>();
        if (winMgr.getInput().checkPressed(hg::in::PK_L, spe::WindowFrameInputView::Mode::Edge)) {
            _documentVisible = !_documentVisible;
            if (_documentVisible) {
                _document->Show();
            } else {
                _document->Hide();
            }
        }
    }

    void setVisible(bool aVisible) {
        _documentVisible = aVisible;
        if (_documentVisible) {
            _document->Show();
        } else {
            _document->Hide();
        }
    }

    void eventDrawGUI() {
#if 0
        const auto& lobbyBackendMgr = ccomp<MLobbyBackend>();

        _lobbyModel.players.resize(hg::pztos(lobbyBackendMgr.getSize()));
        for (hg::PZInteger i = 0; i < lobbyBackendMgr.getSize(); i += 1) {
            const auto& lockedIn = lobbyBackendMgr.getLockedInPlayerInfo(i);
            _lobbyModel.players[hg::pztos(i)].lockedIn =
                PlayerInfoModel{(!lockedIn.name.empty()) ? lockedIn.name : "<empty>",
                                lockedIn.uniqueId,
                                lockedIn.ipAddress};

            if (lobbyBackendMgr.areChangesPending(i)) {
                _lobbyModel.players[hg::pztos(i)].showPending = true;
                const auto& pending                           = lobbyBackendMgr.getPendingPlayerInfo(i);
                _lobbyModel.players[hg::pztos(i)].pending =
                    PlayerInfoModel{(!pending.name.empty()) ? pending.name : "<empty>",
                                    pending.uniqueId,
                                    pending.ipAddress};
            } else {
                _lobbyModel.players[hg::pztos(i)].showPending = false;
            }
        }

        _lobbyModel.localName = lobbyBackendMgr.getLocalName();
        _lobbyModel.isAuthorized =
            CCOMP<spe::AuthorizationManagerInterface>().getLocalAuthToken().has_value();

        _dataModelHandle.DirtyAllVariables();
#endif
    }

private:
    EditorUiDriver& _super;

    AllDefinitions _allDefinitions;

    EditorUiModel        _editorUiModel;
    Rml::DataModelHandle _dataModelHandle;

    Rml::ElementDocument* _document = nullptr;

    bool _documentVisible = false;

    void _onDefinitionSelected(Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& aArguments) {
        const auto index = aArguments.at(0).Get<std::size_t>(-1);
        HG_LOG_INFO(LOG_ID, "Definition {} selected.", index);

        _editorUiModel.selectedIdx = index;
        _dataModelHandle.DirtyVariable("selectedIdx");
    }

    Rml::DataModelHandle _setUpDataBinding(Rml::Context& aContext) {
        Rml::DataModelConstructor constructor;
        RMLUI_DATAMODEL_ASSERT(constructor = aContext.CreateDataModel(DATA_MODEL_NAME));

        // Register types
        RegisterModel<CellDefinitionModel>(constructor);
        RegisterModel<EditorUiModel>(constructor);

        // Bind top-level model
        RMLUI_DATAMODEL_ASSERT(constructor.Bind("cellDefinitions", &_editorUiModel.cellDefinitions));
        RMLUI_DATAMODEL_ASSERT(constructor.Bind("selectedIdx", &_editorUiModel.selectedIdx));
        RMLUI_DATAMODEL_ASSERT(
            constructor.BindEventCallback("SelectDefinition", &Impl::_onDefinitionSelected, this));

        return constructor.GetModelHandle();
    }

    static AllDefinitions _loadAllDefinitions(const std::filesystem::path& aDefinitionsDir) {
        hg::UnicodeString contents;
        if (auto path = aDefinitionsDir / "cells+world.json"; std::filesystem::exists(path)) {
            contents = hg::LoadWholeFile(path);
        } else if (auto path = aDefinitionsDir / "cells+world.txt"; std::filesystem::exists(path)) {
            contents = hg::LoadWholeFile(path);
        } else {
            HG_THROW_TRACED(hg::TracedRuntimeError,
                            0,
                            "Neither 'cells+world.json' nor 'cells+world.txt' were found in '{}'.",
                            aDefinitionsDir.string());
        }

        return JsonStringToAllDefinitions(hg::UniStrConv(hobgoblin::TO_UTF8_STD_STRING, contents));
    }

    static EditorUiModel _allDefinitionsToModel(const AllDefinitions& aDefinitions) {
        EditorUiModel model;

        model.cellDefinitions.reserve(aDefinitions.cellDefinitions.size());
        for (const auto& pair : aDefinitions.cellDefinitions) {
            model.cellDefinitions.push_back({pair.first});
        }

        std::sort(model.cellDefinitions.begin(), model.cellDefinitions.end());

        return model;
    }
};

///////////////////////////////////////////////////////////////////////////
// MARK: EDITOR UI DRIVER                                                //
///////////////////////////////////////////////////////////////////////////

EditorUiDriver::EditorUiDriver(hg::QAO_RuntimeRef aRuntimeRef, int aExecutionPriority)
    : NonstateObject(aRuntimeRef, SPEMPE_TYPEID_SELF, aExecutionPriority, "EditorUiDriver")
    , _impl{std::make_unique<Impl>(*this)} {}

EditorUiDriver::~EditorUiDriver() = default;

void EditorUiDriver::init(const std::filesystem::path& aAssetsDir,
                          const std::filesystem::path& aDefinitionsDir) {
    _impl->init(aAssetsDir, aDefinitionsDir);
}

void EditorUiDriver::_eventBeginUpdate() {
    _impl->eventBeginUpdate();
}

void EditorUiDriver::_eventUpdate1() {
    _impl->eventUpdate1();
}

void EditorUiDriver::_eventDrawGUI() {
    _impl->eventDrawGUI();
}

} // namespace editor
} // namespace gridgoblin
} // namespace jbatnozic