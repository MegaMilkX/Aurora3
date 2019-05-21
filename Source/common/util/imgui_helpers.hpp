#ifndef IMGUI_HELPERS_HPP
#define IMGUI_HELPERS_HPP

#include "../lib/imgui_wrap.hpp"

#include <string>
#include <memory>
#include <functional>
#include "../resource/data_registry.h"
#include "../resource/resource.h"
#include "../resource/resource_factory.h"

#include "../../editor_v2/components/component.hpp"
#include "../../editor_v2/scene/game_object.hpp"
#include "../../editor_v2/scene/game_scene.hpp"

template<typename BASE_T>
inline void imguiHeapObjectCombo(
    const char* label,
    std::shared_ptr<BASE_T>& ptr,
    bool allow_null = true,
    std::function<void(void)> callback = nullptr
) {
    static const char* null_str = "<null>";
    auto derived_array = rttr::type::get<BASE_T>().get_derived_classes();
    if(ImGui::BeginCombo(label, ptr ? ptr->get_type().get_name().to_string().c_str() : null_str)) {
        if(allow_null) {
            if(ImGui::Selectable(null_str, !ptr)) {
                ptr = std::shared_ptr<BASE_T>();
            }
        }
        for(auto d : derived_array) {
            if(ImGui::Selectable(
                d.get_name().to_string().c_str(),
                ptr ? (d == ptr->get_type()) : false
                )
            ) {
                if(!d.is_valid()) {
                    LOG_WARN("Invalid type: " << d.get_name());
                    continue;
                }
                rttr::variant v = d.create();
                if(!v.is_valid() || !v.get_type().is_pointer()) {
                    LOG_WARN("Failed to create value of type " << d.get_name());
                    continue;
                }
                BASE_T* nptr = v.get_value<BASE_T*>();
                ptr.reset(nptr);
                callback();
            }
        }
        
        
        ImGui::EndCombo();
    }
}

template<typename RES_T>
inline void imguiResourceCombo(
    const char* label,
    std::shared_ptr<RES_T>& res,
    const char* ext,
    std::function<void(void)> callback = nullptr
) {
    std::string name = "<null>";
    auto r_list = GlobalDataRegistry().makeList(ext);
    if(res) {
        name = res->Name();
    }
    if(ImGui::BeginCombo(label, name.c_str())) {
        if(ImGui::Selectable("<null>", !res)) {
            res = std::shared_ptr<RES_T>();
        }
        for(auto& rname : r_list) {
            if(ImGui::Selectable(rname.c_str(), strncmp(rname.c_str(), name.c_str(), name.size()) == 0)) {
                res = retrieve<RES_T>(rname);
                if(callback) callback();
            }
        }
        ImGui::EndCombo();
    }
}

template<typename COMP_T>
inline void imguiComponentCombo(
    const char* label,
    COMP_T*& c,
    GameScene* scene,
    std::function<void(void)> callback = nullptr
) {
    std::string name = "<null>";
    if(c) {
        name = c->getOwner()->getName();
    }

    if(ImGui::BeginCombo(label, name.c_str())) {
        auto& list = scene->getAllComponents<COMP_T>();
        if(ImGui::Selectable("<null>", c == 0)) {
            c = 0;
        }
        for(auto& comp : list) {
            if(ImGui::Selectable(comp->getOwner()->getName().c_str(), c == (COMP_T*)comp)) {
                c = (COMP_T*)comp;
                if(callback) callback();
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_OBJECT")) {
            GameObject* tgt_dnd_so = *(GameObject**)payload->Data;
            auto comp = tgt_dnd_so->find<COMP_T>();
            if(comp) {
                c = comp.get();
            }
        }
        ImGui::EndDragDropTarget();
    }
}

template<typename OBJ_T>
inline void imguiObjectCombo(
    const char* label,
    OBJ_T*& o,
    GameScene* scene,
    std::function<void(void)> callback = nullptr
) {
    std::string name = "<null>";
    if(o) {
        name = o->getName();
    }
    if(ImGui::BeginCombo(label, name.c_str())) {
        if(ImGui::Selectable("<null>", o == 0)) {
            o = 0;
        }
        auto& list = scene->getObjects<OBJ_T>();
        for(auto l : list) {
            if(ImGui::Selectable(l->getName().c_str(), o == l)) {
                o = (OBJ_T*)l;
                if(callback) callback();
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_OBJECT")) {
            GameObject* tgt_dnd_so = *(GameObject**)payload->Data;
            if(tgt_dnd_so->get_type() == rttr::type::get<OBJ_T>()) {
                o = (OBJ_T*)tgt_dnd_so;
            } else {
                auto base = tgt_dnd_so->get_type().get_base_classes();
                for(auto bt : base) {
                    if(bt == rttr::type::get<OBJ_T>()) {
                        o = (OBJ_T*)tgt_dnd_so;
                        break;
                    }
                }
            }
        }
        ImGui::EndDragDropTarget();
    }
}

namespace ImGui {

inline bool DragFloat3Autospeed(const char* label, float* v, float v_min = .0f, float v_max = .0f, const char* format = "%.3f", float power = 1.0f) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    int components = 3;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    BeginGroup();
    PushID(label);
    PushMultiItemsWidths(components);
    size_t type_size = sizeof(float);
    for (int i = 0; i < components; i++)
    {
        PushID(i);
        value_changed |= DragScalar(
            "##v", 
            ImGuiDataType_Float, 
            v, 
            std::max(std::abs(*(float*)v * 0.01f), 0.00001f), 
            &v_min, 
            &v_max, 
            format, 
            power
        );
        SameLine(0, g.Style.ItemInnerSpacing.x);
        PopID();
        PopItemWidth();
        v = (float*)((char*)v + type_size);
    }
    PopID();

    TextUnformatted(label, FindRenderedTextEnd(label));
    EndGroup();
    return value_changed;
}

}

#endif