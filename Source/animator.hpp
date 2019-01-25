#ifndef ANIMATOR_HPP
#define ANIMATOR_HPP

#include "component.hpp"
#include "scene_object.hpp"
#include "transform.hpp"

#include "resource/animation.hpp"
#include "resource/skeleton.hpp"
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/base/maths/soa_transform.h>

#include "skeleton_anim_layer.hpp"

class Animator : public Component {
    CLONEABLE
    friend SkeletonAnimLayer;
public:
    Animator() {
        auto& layer0 = addLayer();
        layer0.anim_index = 0;
    }

    ~Animator() {
    }

    void setSkeleton(std::shared_ptr<Skeleton> skel) {
        skeleton = skel;
    }

    void addAnim(std::shared_ptr<Animation> anim) {
        anims.emplace_back(anim);
    }

    SkeletonAnimLayer& addLayer() {
        layers.emplace_back(SkeletonAnimLayer());
        return layers.back();
    }

    void Update(float dt) {
        if(!this->skeleton) {
            return;
        }
        ozz::animation::Skeleton* skeleton = this->skeleton->getOzzSkeleton();

        ozz::animation::SamplingCache* cache = 
            ozz::memory::default_allocator()->New<ozz::animation::SamplingCache>(skeleton->num_joints());
        ozz::Range<ozz::math::SoaTransform> locals_fin = 
            ozz::memory::default_allocator()->AllocateRange<ozz::math::SoaTransform>(skeleton->num_soa_joints());
        ozz::Range<ozz::math::Float4x4> models =
            ozz::memory::default_allocator()->AllocateRange<ozz::math::Float4x4>(skeleton->num_joints());
        
        // Final root motion translation
        gfxm::vec3 rm_pos_final = gfxm::vec3(0.0f, 0.0f, 0.0f);
        // Final root motion rotation
        gfxm::quat rm_rot_final = gfxm::quat(0.0f, 0.0f, 0.0f, 1.0f);

        for(auto& l : layers) {
            l.update(
                this, 
                dt, 
                skeleton, 
                locals_fin, 
                cache, 
                rm_pos_final, 
                rm_rot_final
            );
        }

        ozz::animation::LocalToModelJob ltm_job;
        ltm_job.skeleton = skeleton;
        ltm_job.input = locals_fin;
        ltm_job.output = models;
        if (!ltm_job.Run()) {
            LOG_WARN("Local to model job failed");
            return;
        }

        auto names = skeleton->joint_names();
        for(size_t i = 0; i < names.size() / sizeof(char*); ++i) {
            auto so = getObject()->findObject(names[i]);
            if(!so) continue;
            so->get<Transform>()->setTransform(*(gfxm::mat4*)&models[i]);
        }

        if(root_motion_enabled) {
            Transform* t = getObject()->get<Transform>();
            
            t->translate(rm_pos_final);
            t->rotate(rm_rot_final);
        }

        ozz::memory::default_allocator()->Deallocate(locals_fin);
        ozz::memory::default_allocator()->Deallocate(models);
        ozz::memory::default_allocator()->Delete(cache);
    }

    size_t selected_index = 0;
    virtual void _editorGui() {
        ImGui::Checkbox("Enable root motion", &root_motion_enabled);

        ImGui::Text("Layers");
        ImGui::BeginChild("Layers", ImVec2(0, 100), false, 0);
        
        for(size_t i = 0; i < layers.size(); ++i) {
            if(ImGui::Selectable(MKSTR("Layer " << i).c_str(), selected_index == i)) {
                selected_index = i;
            }
        }
        ImGui::EndChild();
        if(ImGui::Button("Add")) {
            addLayer();
            selected_index = layers.size() - 1;
        } ImGui::SameLine(); 
        if(ImGui::Button("Remove") && !layers.empty()) {
            layers.erase(layers.begin() + selected_index);
            if(selected_index >= layers.size()) {
                selected_index = layers.size() - 1;
            }
        }
        if(!layers.empty()) {
            std::string current_anim_name = "";
            if(!anims.empty()) {
                current_anim_name = anims[layers[selected_index].anim_index]->Name();
            }

            if (ImGui::BeginCombo("Current anim", current_anim_name.c_str(), 0)) {
                for(size_t i = 0; i < anims.size(); ++i) {
                    if(ImGui::Selectable(anims[i]->Name().c_str(), layers[selected_index].anim_index == i)) {
                        layers[selected_index].anim_index = i;
                    }
                }
                ImGui::EndCombo();
            }

            if(ImGui::BeginCombo("Mode", SkeletonAnimLayer::blendModeString(layers[selected_index].mode).c_str())) {
                for(size_t i = 0; i < SkeletonAnimLayer::BLEND_MODE_LAST; ++i) {
                    if(ImGui::Selectable(SkeletonAnimLayer::blendModeString((SkeletonAnimLayer::BlendMode)i).c_str(), layers[selected_index].mode == i)) {
                        layers[selected_index].mode = (SkeletonAnimLayer::BlendMode)i;
                    }
                }
                ImGui::EndCombo();
            }
            ImGui::SliderFloat("Cursor", &layers[selected_index].cursor, 0.0f, 1.0f);
            if(ImGui::DragFloat("Speed", &layers[selected_index].speed, 0.01f, 0.0f, 10.0f)) {}
            if(ImGui::DragFloat("Weight", &layers[selected_index].weight, 0.01f, 0.0f, 1.0f)) {}
            bool looping = true;
            if(ImGui::Checkbox("Looping", &looping)) {
            } ImGui::SameLine();
            bool autoplay = true;
            if(ImGui::Checkbox("Autoplay", &autoplay)) {
            }
        }
        ImGui::Separator();

        for(auto a : anims) {
            bool selected = false;
            ImGui::Selectable(a->Name().c_str(), &selected);
        }
        //ImGui::ListBox("Animations", 0, anim_list.data(), anim_list.size(), 4);
    }
private:
    bool root_motion_enabled = true;
    std::vector<SkeletonAnimLayer> layers;
    std::vector<std::shared_ptr<Animation>> anims;
    std::shared_ptr<Skeleton> skeleton;
};
STATIC_RUN(Animator)
{
    rttr::registration::class_<Animator>("Animator")
        .constructor<>()(
            rttr::policy::ctor::as_raw_ptr
        );
}

#endif