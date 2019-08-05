#ifndef IMGUI_HELPERS_HPP
#define IMGUI_HELPERS_HPP

#include "../lib/imgui_wrap.hpp"

#include <string>
#include <memory>
#include <functional>
#include "../resource/data_registry.h"
#include "../resource/resource.h"
#include "../resource/resource_tree.hpp"

#include "../components/component.hpp"
#include "../scene/node.hpp"
#include "../scene/game_scene.hpp"

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

template<typename T>
void imguiResourceTreeCombo(
    const char* label,
    std::shared_ptr<T>& res,
    const char* ext,
    std::function<void(void)> callback = nullptr
) {
    std::string current_name = "<null>";
    if(res) {
        current_name = res->Name();
    }
    if(ImGui::BeginCombo(label, current_name.c_str())) {
        std::set<const ResourceNode*> valid_nodes;
        std::function<bool(const std::shared_ptr<ResourceNode>&, const std::string&, std::set<const ResourceNode*>&)> walkNodes;
        walkNodes = [&walkNodes](const std::shared_ptr<ResourceNode>& node, const std::string& suffix,  std::set<const ResourceNode*>& valid_nodes)->bool {
            bool has_valid_child = false;
            for(auto& kv : node->getChildren()) {
                if(walkNodes(kv.second, suffix, valid_nodes)) {
                    has_valid_child = true;
                }
            }
            if(!has_valid_child) {
                if(has_suffix(node->getName(), suffix)) {
                    valid_nodes.insert(node.get());
                    return true;
                }
            } else {
                valid_nodes.insert(node.get());
                return true;
            }
            return false;
        };

        walkNodes(gResourceTree.getRoot(), MKSTR("." << ext), valid_nodes);

        std::function<void(const std::shared_ptr<ResourceNode>&, const std::set<const ResourceNode*>&)> imguiResourceTree;
        imguiResourceTree = [&callback, &res, &imguiResourceTree](const std::shared_ptr<ResourceNode>& node, const std::set<const ResourceNode*>& valid_nodes) {
            if(valid_nodes.find(node.get()) == valid_nodes.end()) {
                return;
            }
            std::string node_label = node->getName();
            if(node->isLoaded()) {
                node_label += " [L]";
            }
            if(node->childCount()) {
                if(ImGui::TreeNodeEx(
                    (void*)node.get(),
                    ImGuiTreeNodeFlags_OpenOnDoubleClick |
                    ImGuiTreeNodeFlags_OpenOnArrow,
                    node_label.c_str()
                )) {
                    for(auto& kv : node->getChildren()) {
                        imguiResourceTree(kv.second, valid_nodes);
                    }
                    ImGui::TreePop();
                }
            } else {
                if(ImGui::Selectable(node_label.c_str())) {
                    res = node->getResource<T>();
                    if(callback) callback();
                    //setClip(node->getResource<AudioClip>());
                }
            }
        };

        for(auto& kv : gResourceTree.getRoot()->getChildren()) {
            imguiResourceTree(kv.second, valid_nodes);
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
    // TODO: FIX
    auto r_list = std::vector<std::string>(); //GlobalDataRegistry().makeList(ext);
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
            ktNode* tgt_dnd_so = *(ktNode**)payload->Data;
            auto comp = tgt_dnd_so->find<COMP_T>();
            if(comp) {
                c = comp.get();
            }
        }
        ImGui::EndDragDropTarget();
    }
}

inline void imguiObjectCombo(
    const char* label,
    ktNode*& o,
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
        std::vector<ktNode*> list;
        scene->getRoot()->getAllObjects(list);
        for(auto l : list) {
            if(ImGui::Selectable(l->getName().c_str(), o == l)) {
                o = l;
                if(callback) callback();
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_OBJECT")) {
            ktNode* tgt_dnd_so = *(ktNode**)payload->Data;
            o = tgt_dnd_so;
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
