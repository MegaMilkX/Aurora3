#include "skeleton_anim_layer.hpp"

#include "animator.hpp"

#include "transform.hpp"

#include "scene.hpp"

static void blendAnim(
    Animation* anim,
    std::map<size_t, size_t>& bone_remap,
    float cursor, 
    float cursor_prev,
    ANIM_BLEND_MODE mode,
    float weight,
    std::vector<AnimSample>& samples,
    bool enable_root_motion,
    Transform* root_transform,
    gfxm::vec3& rm_pos_final,
    gfxm::quat& rm_rot_final
) {
    gfxm::vec3 root_motion_pos_delta;
    gfxm::quat root_motion_rot_delta;
    bool do_root_motion = anim->root_motion_enabled && root_transform;
    if(do_root_motion) {
        gfxm::vec3 delta_pos =  anim->getRootMotionNode().t.delta(cursor_prev, cursor);
        gfxm::vec3 delta_pos4 = gfxm::vec4(delta_pos.x, delta_pos.y, delta_pos.z, 0.0f);
        gfxm::quat delta_q = anim->getRootMotionNode().r.delta(cursor_prev, cursor);
        if(root_transform->parentTransform()) {}

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

void SkeletonAnimLayer::update(
    Animator* animator,
    float dt,
    Skeleton* skeleton,
    std::vector<AnimSample>& t_fin,
    gfxm::vec3& rm_pos_final,
    gfxm::quat& rm_rot_final
) {
    if(weight == 0.0f) {
        return;
    }
    if(anim_index >= animator->anims.size()) {
        return;
    }
    auto& anim_info = animator->anims[anim_index];
    auto& anim = anim_info.anim;

    float duration = anim_info.anim->length;
    float fps      = anim_info.anim->fps;

    float cursor_prev = cursor;
    if(!stopped) {
        cursor += dt * fps * speed;
        if(cursor >= duration) {
            if(anim_info.looping) {
                cursor -= duration;
            } else {
                cursor = duration;
                stopped = true;
            }
        }
    }

    // TODO: Optimize
    Transform* root_motion_transform = 0;
    if(!anim->root_motion_node_name.empty()) {
        SceneObject* root_so = animator->getObject()->findObject(anim->root_motion_node_name);
        if(root_so) {
            root_motion_transform = 
                root_so->get<Transform>();
        }
    }

    blendAnim(
        anim_info.anim.get(),
        anim_info.bone_remap,
        cursor, cursor_prev,
        mode,
        weight,
        t_fin,
        animator->root_motion_enabled,
        root_motion_transform,
        rm_pos_final,
        rm_rot_final
    );

    if(blend_over_speed > 0.0f) {
        float fps = animator->anims[blend_target_index].anim->fps;
        blend_over_weight += dt * blend_over_speed;
        if(blend_over_weight > 1.0f) blend_over_weight = 1.0f;

        blendAnim(
            animator->anims[blend_target_index].anim.get(),
            animator->anims[blend_target_index].bone_remap,
            blend_target_cursor, blend_target_prev_cursor,
            ANIM_MODE_BLEND,
            blend_over_weight,
            t_fin,
            animator->root_motion_enabled,
            root_motion_transform,
            rm_pos_final,
            rm_rot_final
        );

        blend_target_prev_cursor = blend_target_cursor;
        blend_target_cursor += dt * fps * speed;
        if(blend_over_weight >= 1.0f) {
            anim_index = blend_target_index;
            cursor = blend_target_cursor;
            blend_over_speed = 0.0f;
            stopped = false;
        }
    }
}

/*
void SkeletonAnimLayer::update(
    Animator* animator,
    float dt,
    ozz::animation::Skeleton* skeleton,
    ozz::Range<ozz::math::SoaTransform>& locals_fin,
    ozz::animation::SamplingCache* cache,
    gfxm::vec3& rm_pos_final,
    gfxm::quat& rm_rot_final
) {/*
    if(anim_index >= animator->anims.size()) {
        return;
    }
    std::shared_ptr<Animation> anim = animator->anims[anim_index];
    if(anim->anim->duration() == 0.0f) {
        return;
    }
    ozz::Range<ozz::math::SoaTransform> locals = 
        ozz::memory::default_allocator()->AllocateRange<ozz::math::SoaTransform>(skeleton->num_soa_joints());
    float cursor_prev = cursor;
    cursor += dt * 60.0f;
    if(cursor > anim->anim->duration()) {
        cursor -= anim->anim->duration();
    }

    Transform* root_motion_transform = 0;
    if(!anim->root_motion_node.empty()) {
        SceneObject* root_so = animator->getObject()->findObject(anim->root_motion_node);
        if(root_so) {
            root_motion_transform = 
                root_so->get<Transform>();
        }
    }

    ozz::animation::SamplingJob sampling_job;
    sampling_job.animation = anim->anim;
    sampling_job.cache = cache;
    sampling_job.output = locals;
    sampling_job.ratio = cursor / anim->anim->duration();

    if(!sampling_job.Run()) {
        LOG_WARN("Sample job failed");
        return;
    }

    gfxm::vec3 root_motion_pos_delta;
    gfxm::quat root_motion_rot_delta;
    bool do_root_motion = anim->root_motion_enabled && root_motion_transform;
    if(do_root_motion) {
        gfxm::vec3 delta_pos = anim->root_motion_pos.delta(cursor_prev, cursor);
        gfxm::vec3 delta_pos4 = gfxm::vec4(delta_pos.x, delta_pos.y, delta_pos.z, 1.0f);
        gfxm::quat delta_q = anim->root_motion_rot.delta(cursor_prev, cursor);
        if(root_motion_transform->parentTransform()) {
            delta_pos4 = root_motion_transform->getParentTransform() * delta_pos4;
            gfxm::quat w_rot = root_motion_transform->worldRotation();
        }
        root_motion_pos_delta = gfxm::vec3(delta_pos4.x, delta_pos4.y, delta_pos4.z);
        root_motion_rot_delta = gfxm::inverse(root_motion_transform->rotation()) * delta_q * root_motion_transform->rotation();
    }

    switch(mode) {
    case BASE:
        memcpy(locals_fin.begin, locals.begin, locals_fin.size());
        if(animator->root_motion_enabled) {
            rm_pos_final = root_motion_pos_delta;
            rm_rot_final = root_motion_rot_delta;
        }
        break;
    case BLEND: 
        {
            ozz::Range<ozz::math::SoaTransform> locals_blend = 
                ozz::memory::default_allocator()->AllocateRange<ozz::math::SoaTransform>(skeleton->num_soa_joints());
        
            ozz::animation::BlendingJob::Layer layers[2];
            layers[0].transform = locals_fin;
            layers[0].weight = 1.0f - weight;
            layers[1].transform = locals;
            layers[1].weight = weight;
            ozz::animation::BlendingJob blend_job;
            blend_job.threshold = 1.0f;
            blend_job.layers = layers;
            blend_job.bind_pose = skeleton->bind_pose();
            blend_job.output = locals_blend;

            // Blends.
            if (!blend_job.Run()) {
                LOG_WARN("Blending job failed");
                return;
            }
            memcpy(locals_fin.begin, locals_blend.begin, locals_fin.size());
            ozz::memory::default_allocator()->Deallocate(locals_blend);

            if(animator->root_motion_enabled) {
                rm_pos_final = gfxm::lerp(rm_pos_final, root_motion_pos_delta, weight);
                rm_rot_final = gfxm::slerp(rm_rot_final, root_motion_rot_delta, weight);
            }
        }
        break;
    case ADD:
        {
            ozz::Range<ozz::math::SoaTransform> locals_blend = 
                ozz::memory::default_allocator()->AllocateRange<ozz::math::SoaTransform>(skeleton->num_soa_joints());
        
            ozz::animation::BlendingJob::Layer layers[1];
            layers[0].transform = locals_fin;
            layers[0].weight = 1.0f;
            ozz::animation::BlendingJob::Layer layers_add[1];
            layers_add[0].transform = locals;
            layers_add[0].weight = weight;

            ozz::animation::BlendingJob blend_job;
            blend_job.threshold = 1.0f;
            blend_job.layers = layers;
            blend_job.additive_layers = layers_add;
            blend_job.bind_pose = skeleton->bind_pose();
            blend_job.output = locals_blend;

            // Blends.
            if (!blend_job.Run()) {
                LOG_WARN("Blending job failed");
                return;
            }
            memcpy(locals_fin.begin, locals_blend.begin, locals_fin.size());
            ozz::memory::default_allocator()->Deallocate(locals_blend);

            // NOTE: Additive root motion is untested
            if(animator->root_motion_enabled) {
                rm_pos_final = gfxm::lerp(rm_pos_final, rm_pos_final + root_motion_pos_delta, weight);
                rm_rot_final = gfxm::slerp(rm_rot_final, root_motion_rot_delta * rm_rot_final, weight);
            }
        }
        break;
    };
    
    ozz::memory::default_allocator()->Deallocate(locals);    
}
*/