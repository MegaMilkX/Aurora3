#include "editor_doc_scene.hpp"

#include "../common/scene/game_scene.hpp"

#include "../common/scene/controllers/render_controller.hpp"
#include "../common/scene/controllers/dynamics_ctrl.hpp"
#include "../common/scene/controllers/constraint_ctrl.hpp"
#include "../common/scene/controllers/audio_controller.hpp"
#include "../common/scene/controllers/anim_controller.hpp"

#include "editor.hpp"

EditorDocScene::EditorDocScene() {
    bindActionPress("ALT", [this](){ 
        gvp.camMode(GuiViewport::CAM_ORBIT); 
    });
    bindActionRelease("ALT", [this](){ gvp.camMode(GuiViewport::CAM_PAN); });
}
EditorDocScene::EditorDocScene(std::shared_ptr<ResourceNode>& node)
: EditorDocScene() {
    setResourceNode(node);
    gvp.enableDebugDraw(true);    
}

void EditorDocScene::onGui (Editor* ed) {
    auto& scene = _resource;

    ImGuiID dock_id = ImGui::GetID(getName().c_str());
    ImGui::DockSpace(dock_id);

    const std::string win_vp_name = MKSTR("viewport##" << getName());
    const std::string win_scene_insp_name = MKSTR("Scene Inspector##" << getName());
    const std::string win_obj_insp_name = MKSTR("Object Inspector##" << getName());

    if(first_use) {
        ImGuiID dsid_right = ImGui::DockBuilderSplitNode(dock_id, ImGuiDir_Right, 0.2f, NULL, &dock_id);
        ImGuiID dsid_right_bottom = ImGui::DockBuilderSplitNode(dsid_right, ImGuiDir_Down, 0.5f, NULL, &dsid_right);

        ImGui::DockBuilderDockWindow(win_vp_name.c_str(), dock_id);
        ImGui::DockBuilderDockWindow(win_obj_insp_name.c_str(), dsid_right);
        ImGui::DockBuilderDockWindow(win_scene_insp_name.c_str(), dsid_right_bottom);

        first_use = false;
    }

    bool open = true;
    if(ImGui::Begin(win_vp_name.c_str(), &open, ImVec2(200, 200))) {
        if(ImGui::IsRootWindowOrAnyChildFocused()) {
            // TODO: Shouldn't be here
            ed->getResourceTree().setSelected(getNode());
            ed->setFocusedDocument(this);
        }

        scene->getController<AnimController>()->onUpdate();
        gvp.draw(scene.get(), 0, gfxm::ivec2(0,0));
        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_DND_RESOURCE)) {
                ResourceNode* node = *(ResourceNode**)payload->Data;
                LOG("Payload received: " << node->getFullName());
                if(has_suffix(node->getName(), ".so")) {
                    auto o = scene->createInstance(node->getResource<GameScene>());

                    gfxm::vec3 pos = gvp.getMouseScreenToWorldPos(0);
                    pos = gfxm::inverse(scene->getTransform()->getWorldTransform()) * gfxm::vec4(pos, 1.0f);
                    gfxm::vec3 scale_new = o->getTransform()->getScale();
                    gfxm::vec3 scale_world = scene->getTransform()->getScale();
                    o->getTransform()->setScale(gfxm::vec3(scale_new.x / scale_world.x, scale_new.y / scale_world.y, scale_new.z / scale_world.z));
                    o->getTransform()->translate(pos);
                }
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::End();
    }

    scene_inspector.update(scene.get(), selected, win_scene_insp_name);
    object_inspector.update(scene.get(), selected, win_obj_insp_name);

    for(auto& o : selected.getAll()) {
        o->onGizmo(gvp);
    }
}