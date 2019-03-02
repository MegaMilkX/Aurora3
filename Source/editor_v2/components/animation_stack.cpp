#include "animation_stack.hpp"

#include "../scene/scene_object.hpp"

void AnimationStack::setSkeleton(std::shared_ptr<Skeleton> skel) {
    skeleton = skel;
    resetSampleBuffer();
    resetSkeletonMapping();
}
void AnimationStack::addAnim(std::shared_ptr<Animation> anim) {
    anims.emplace_back(
        AnimInfo {
            anim->Name().substr(anim->Name().find_last_of("/")),
            anim
        }
    );
    resetSkeletonMapping(anims.back());
}
void AnimationStack::reserveLayers(unsigned count) {
    layers.resize(count);
}
AnimLayer& AnimationStack::addLayer() {
    layers.emplace_back(AnimLayer());
    return layers.back();
}
AnimLayer& AnimationStack::getLayer(unsigned i) {
    return layers[i];
}

void AnimationStack::update(float dt) {
    if(!skeleton) return;

    gfxm::vec3 root_motion_trans = gfxm::vec3(.0f, .0f, .0f);
    gfxm::quat root_motion_rot = gfxm::quat(.0f, .0f, .0f, 1.0f);

    for(auto& l : layers) {
        updateLayer(
            l, dt,
            root_motion_trans,
            root_motion_rot
        );
    }

    // Update transforms
    // TODO: Optimize
    for(size_t i = 0; i < skeleton->boneCount(); ++i) {
        Skeleton::Bone& b = skeleton->getBone(i);
        auto so = getOwner()->findObject(b.name);
        if(!so) continue;
        auto trans = so->getTransform();
        trans->setPosition(samples[i].t);
        trans->setRotation(samples[i].r);
        trans->setScale(samples[i].s);
    }

    // Apply root motion
    if(root_motion_enabled) {
        auto t = getOwner()->getTransform();

        t->rotate(root_motion_rot);

        gfxm::vec4 t4 = gfxm::vec4(
            root_motion_trans.x,
            root_motion_trans.y,
            root_motion_trans.z,
            0.0f
        );
        gfxm::mat4 root_m4 = t->getWorldTransform();
        root_m4[3] = gfxm::vec4(.0f, .0f, .0f, 1.0f);
        root_m4[0] = gfxm::normalize(root_m4[0]);
        root_m4[1] = gfxm::normalize(root_m4[1]);
        root_m4[2] = gfxm::normalize(root_m4[2]);
        t4 = (root_m4) * t4;

        t->translate(t4);
    }
}

// ==== Private ===================

void AnimationStack::updateLayer(
    AnimLayer& l, 
    float dt,
    gfxm::vec3& rm_pos_final,
    gfxm::quat& rm_rot_final
) {
    if(l.weight == 0.0f) return;
    if(l.anim_index >= anims.size()) return;

    auto& anim_info = anims[l.anim_index];
    auto& anim = anim_info.anim;
    float duration = anim_info.anim->length;
    float fps = anim_info.anim->fps;

    float cursor_prev = l.cursor;
    if(!l.stopped) {
        l.cursor += dt * fps * l.speed;
        if(l.cursor >= duration) {
            if(anim_info.looping) {
                l.cursor -= duration;
            } else {
                l.cursor = duration;
                l.stopped = true;
            }
        }
    }

    blendAnim(
        anim_info.anim.get(),
        anim_info.bone_remap,
        l.cursor, cursor_prev,
        l.mode, l.weight,
        samples,
        root_motion_enabled,
        rm_pos_final,
        rm_rot_final
    );

    if(l.blend_over_speed > 0.0f) {
        float fps = anims[l.blend_target_index].anim->fps;
        l.blend_over_weight += dt * l.blend_over_speed;
        if(l.blend_over_weight > 1.0f) l.blend_over_weight = 1.0f;

        blendAnim(
            anims[l.blend_target_index].anim.get(),
            anims[l.blend_target_index].bone_remap,
            l.blend_target_cursor, l.blend_target_prev_cursor,
            ANIM_MODE_BLEND, l.blend_over_weight,
            samples,
            root_motion_enabled,
            rm_pos_final,
            rm_rot_final
        );

        l.blend_target_prev_cursor = l.blend_target_cursor;
        l.blend_target_cursor += dt * fps * l.speed;
        if(l.blend_over_weight >= 1.0f) {
            l.anim_index = l.blend_target_index;
            l.cursor = l.blend_target_cursor;
            l.blend_over_speed = 0.0f;
            l.stopped = false;
        }
    }
}

void AnimationStack::blendAnim(
    Animation* anim,
    std::vector<size_t>& bone_remap,
    float cursor, 
    float cursor_prev,
    ANIM_BLEND_MODE mode,
    float weight,
    std::vector<AnimSample>& samples,
    bool enable_root_motion,
    gfxm::vec3& rm_pos_final,
    gfxm::quat& rm_rot_final
) {
    gfxm::vec3 root_motion_pos_delta;
    gfxm::quat root_motion_rot_delta;
    bool do_root_motion = anim->root_motion_enabled;
    if(do_root_motion) {
        gfxm::vec3 delta_pos =  anim->getRootMotionNode().t.delta(cursor_prev, cursor);
        gfxm::vec3 delta_pos4 = gfxm::vec4(delta_pos.x, delta_pos.y, delta_pos.z, 0.0f);
        gfxm::quat delta_q = anim->getRootMotionNode().r.delta(cursor_prev, cursor);

        root_motion_pos_delta = delta_pos4;
        root_motion_rot_delta = delta_q;
    }

    switch(mode) {
    case ANIM_MODE_NONE:
        anim->sample_remapped(samples, cursor, bone_remap);
        if(enable_root_motion) {
            rm_pos_final = root_motion_pos_delta;
            rm_rot_final = root_motion_rot_delta;
        }
        break;
    case ANIM_MODE_BLEND:
        anim->blend_remapped(samples, cursor, weight, bone_remap);
        if(enable_root_motion) {
            rm_pos_final = gfxm::lerp(rm_pos_final, root_motion_pos_delta, weight);
            rm_rot_final = gfxm::slerp(rm_rot_final, root_motion_rot_delta, weight);
        }
        break;
    case ANIM_MODE_ADD:
        anim->additive_blend_remapped(samples, cursor, weight, bone_remap);
        // NOTE: Additive root motion is untested
        if(enable_root_motion) {
            rm_pos_final = gfxm::lerp(rm_pos_final, rm_pos_final + root_motion_pos_delta, weight);
            rm_rot_final = gfxm::slerp(rm_rot_final, root_motion_rot_delta * rm_rot_final, weight);
        }
        break;
    }
}

void AnimationStack::resetSkeletonMapping() {
    if(!skeleton) return;
    for(auto& a : anims) {
        resetSkeletonMapping(a);
    }
}
void AnimationStack::resetSkeletonMapping(AnimInfo& anim_info) {
    if(!skeleton) return;

    anim_info.bone_remap.resize(anim_info.anim->nodeCount());
    for(size_t i = 0; i < skeleton->boneCount(); ++i) {
        Skeleton::Bone& b = skeleton->getBone(i);
        int32_t bone_index = (int32_t)i;
        int32_t node_index = anim_info.anim->getNodeIndex(b.name);
        if(node_index >= 0) {
            anim_info.bone_remap[node_index] = bone_index;
        }
    }
}
void AnimationStack::resetSampleBuffer() {
    if(!skeleton) return;
    samples.resize(skeleton->boneCount());
    for(size_t i = 0; i < skeleton->boneCount(); ++i) {
        Skeleton::Bone& b = skeleton->getBone(i);

        gfxm::vec3 _position = gfxm::vec3(b.bind_pose[3].x, b.bind_pose[3].y, b.bind_pose[3].z);
        gfxm::mat3 rotMat = gfxm::to_orient_mat3(b.bind_pose);
        gfxm::quat _rotation = gfxm::to_quat(rotMat);
        gfxm::vec3 right = b.bind_pose[0];
        gfxm::vec3 up = b.bind_pose[1];
        gfxm::vec3 back = b.bind_pose[2];
        gfxm::vec3 _scale = gfxm::vec3(right.length(), up.length(), back.length());

        samples[i].t = _position;
        samples[i].r = _rotation;
        samples[i].s = _scale;
    }
}