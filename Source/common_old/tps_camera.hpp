#ifndef TPS_CAMERA_HPP
#define TPS_CAMERA_HPP

#include "component.hpp"
#include "camera.hpp"
#include "scene_object.hpp"
#include "behavior_system.hpp"
#include "input/input_mgr.hpp"

#include "scene_components/scene_player_info.hpp"
#include "scene_components/scene_physics_world.hpp"
#include "scene.hpp"

class TpsCamera : public Updatable {
    CLONEABLE_AUTO
    RTTR_ENABLE(Updatable)
public:
    virtual void onCreate() {
        camera = getObject()->get<Camera>();
        transform = getObject()->get<Transform>();

        player_info = getScene()->getSceneComponent<ScenePlayerInfo>();

        physics_world = getScene()->getSceneComponent<PhysicsWorld>();
    }

    virtual void init() {
        camera->makeCurrent();

        input_lis = input().createListener();
        
        input_lis->bindAxis(
            "MoveCamX",
            [this](float v) {
                angle_y -= v * 0.01f;
            }
        );
        input_lis->bindAxis(
            "MoveCamY",
            [this](float v) {
                angle_x -= v * 0.01f;
                if(angle_x > gfxm::pi * 0.10f) {
                    angle_x = gfxm::pi * 0.10f;
                } else if(angle_x < -gfxm::pi * 0.45f) {
                    angle_x = -gfxm::pi * 0.45f;
                }
            }
        );
        input_lis->bindAxis(
            "CameraZoom",
            [this](float v) {
                float mod = distance;
                distance += -v * mod * 0.15f;
                if(distance < 1.0f) {
                    distance = 1.0f;
                } else if(distance > 3.0f) {
                    distance = 3.0f;
                }
            }
        );
    }

    virtual void cleanup() {
        input().removeListener(input_lis);
    }

    virtual void update() {
        Transform* t = player_info->character_transform;
        gfxm::vec3 tgt_pos;

        if(t) {
            tgt_pos = t->worldPosition() + gfxm::vec3(0.0f, 1.6f, 0.0f);
        } else {
            tgt_pos = gfxm::vec3(0.0,0.0,0.0);
        }

        pivot = gfxm::lerp(pivot, tgt_pos, std::pow(gfxm::clamp(gfxm::length(tgt_pos - pivot), 0.0f, 1.0f), 2.0f));

        _distance = gfxm::lerp(_distance, distance, 0.1f);
        _angle_y = gfxm::lerp(_angle_y, angle_y, 0.1f);
        _angle_x = gfxm::lerp(_angle_x, angle_x, 0.1f);
        gfxm::transform tcam;
        tcam.position(pivot);
        tcam.rotate(_angle_y, gfxm::vec3(0.0f, 1.0f, 0.0f));
        tcam.rotate(_angle_x, tcam.right());

        gfxm::vec3 hit;
        if(physics_world->sphereSweepClosestHit(0.3f, pivot, pivot + tcam.back() * _distance, collision_flags, hit)) {
            float len = gfxm::length(pivot - hit);
            if(len < distance) {
                _distance = len;
            }
        }

        tcam.translate(tcam.back() * _distance);

        transform->setTransform(tcam.matrix());
    }

    virtual void _editorGui() {
        ImGui::Text("Collision masking");
        ImGui::CheckboxFlags("Static", &collision_flags, 1);
        ImGui::CheckboxFlags("Probe", &collision_flags, 2);
        ImGui::CheckboxFlags("Sensor", &collision_flags, 4);
        ImGui::CheckboxFlags("Hitbox", &collision_flags, 8);
        ImGui::CheckboxFlags("Hurtbox", &collision_flags, 16);
    }

    virtual void serialize(std::ostream& out) {
        write(out, (uint64_t)collision_flags);
    }
    virtual void deserialize(std::istream& in, size_t sz) {
        collision_flags = (uint32_t)read<uint64_t>(in);
    }
private:
    unsigned collision_flags = 0;

    InputListener* input_lis;

    ScenePlayerInfo* player_info;

    PhysicsWorld* physics_world = 0;

    Camera* camera = 0;
    Transform* transform = 0;
    gfxm::vec3 pivot = gfxm::vec3(0.0f, 1.3f, 0.0f);
    float distance = 10.0f;
    float angle_y = 0.0f;
    float angle_x = 0.0f;
    float _distance = 10.0f;
    float _angle_y = 0.0f;
    float _angle_x = 0.0f;
};
STATIC_RUN(TpsCamera)
{
    rttr::registration::class_<TpsCamera>("TpsCamera")
        .constructor<>()(
            rttr::policy::ctor::as_raw_ptr
        );
}

#endif
