#ifndef COLLISION_SHAPES_HPP
#define COLLISION_SHAPES_HPP

#include <btBulletDynamicsCommon.h>

#include "../../common/util/reg_type.hpp"

#include "../../common/util/imgui_helpers.hpp"

#include "../../common/resource/mesh.hpp"
#include "model.hpp"

#include "../scene/game_object.hpp"

class BaseShape_ {
    RTTR_ENABLE()
public:
    void setDirty() { dirty = true; }
    bool isDirty() const { return dirty; }

    virtual bool onGui(GameObject* o) = 0;

    virtual void serialize(out_stream& out) {}
    virtual void deserialize(in_stream& in) {}

    btCollisionShape* getBtShape() { return bt_shape.get(); }
protected:
    bool dirty = true;
    std::shared_ptr<btCollisionShape> bt_shape;
};
class ConvexShape_ : public BaseShape_ {
    RTTR_ENABLE(BaseShape_)
};
class ConcaveShape_ : public BaseShape_ {
    RTTR_ENABLE(BaseShape_)
};

class EmptyShape_ : public BaseShape_ {
    RTTR_ENABLE(BaseShape_)
public:
    EmptyShape_() {
        bt_shape.reset(new btEmptyShape());
    }
    virtual bool onGui(GameObject* o) {
        return false;
    }
};
class SphereShape_ : public ConvexShape_ {
    RTTR_ENABLE(ConvexShape_)
public:
    SphereShape_() {
        bt_shape.reset(new btSphereShape(radius));
    }
    virtual bool onGui(GameObject* o) {
        auto s = (btSphereShape*)bt_shape.get();
        if(ImGui::DragFloat("radius", &radius, radius * 0.01f)) {
            *s = btSphereShape(radius);
        }
        return false;
    }
    virtual void serialize(out_stream& out) {
        auto s = (btSphereShape*)bt_shape.get();
        out.write(radius);
    }
    virtual void deserialize(in_stream& in) {
        auto s = (btSphereShape*)bt_shape.get();
        radius = in.read<float>();
        *s = btSphereShape(radius);
    }
private:
    float radius = 1.0f;
};
class BoxShape_ : public ConvexShape_ {
    RTTR_ENABLE(ConvexShape_)
public:
    BoxShape_() {
        bt_shape.reset(new btBoxShape(btVector3(extents.x,extents.y,extents.z)));
    }
    virtual bool onGui(GameObject* o) {
        auto s = (btBoxShape*)bt_shape.get();
        if(ImGui::DragFloat3("extents", (float*)&extents, 0.01f)) {
            *s = btBoxShape(btVector3(extents.x, extents.y, extents.z));
        }
        return false;
    }
    virtual void serialize(out_stream& out) {
        out.write(extents);
    }
    virtual void deserialize(in_stream& in) {
        auto s = (btBoxShape*)bt_shape.get();
        gfxm::vec3 extents = in.read<gfxm::vec3>();
        *s = btBoxShape(btVector3(extents.x, extents.y, extents.z));
    }
private:
    gfxm::vec3 extents = gfxm::vec3(1,1,1);
};
class CapsuleShape_ : public ConvexShape_ {
    RTTR_ENABLE(ConvexShape_)
public:
    CapsuleShape_() {
        bt_shape.reset(new btCapsuleShape(radius, height));
    }
    virtual bool onGui(GameObject* o) {
        auto s = (btCapsuleShape*)bt_shape.get();
        if(ImGui::DragFloat("radius", &radius, std::max(radius * 0.01f, 0.01f))) {
            *s = btCapsuleShape(radius, height);
        }
        if(ImGui::DragFloat("height", &height, std::max(height * 0.01f, 0.01f))) {
            *s = btCapsuleShape(radius, height);
        }
        return false;
    }
    virtual void serialize(out_stream& out) {
        out.write(radius);
        out.write(height);
    }
    virtual void deserialize(in_stream& in) {
        auto s = (btCapsuleShape*)bt_shape.get();
        radius = in.read<float>();
        height = in.read<float>();
        *s = btCapsuleShape(radius, height);
    }
private:
    float radius = .5f;
    float height = 1.0f;
};

// ============

class MeshShape_ : public ConcaveShape_ {
    RTTR_ENABLE(ConcaveShape_)
public:
    MeshShape_() {
        empty.reset(new btEmptyShape());
        bt_shape = empty;
    }
    virtual bool onGui(GameObject* o) {
        bool need_rebuild = false;
        ImGui::TextWrapped("Debug display for collision meshes is disabled for performance");
        imguiResourceCombo<Mesh>(
            "mesh",
            mesh,
            ".msh",
            [this, &need_rebuild](){
                setMesh(mesh);
                need_rebuild = true;
            }
        );
        if(ImGui::Button("Make from model")) {
            auto mdl = o->find<Model>();
            if(mdl) {
                makeFromModel(mdl);
            }
            need_rebuild = true;
        }
        return need_rebuild;
    }
private:
    void makeFromModel(std::shared_ptr<Model> mdl) {
        if(!mdl) return;
        if(mdl->segmentCount() == 0) return;
        auto& seg = mdl->getSegment(0);
        setMesh(seg.mesh);
    }
    void setMesh(const std::string& res_name) {
        setMesh(retrieve<Mesh>(res_name));
    }
    void setMesh(std::shared_ptr<Mesh> mesh) {
        if(!mesh) return;
        if(!(mesh->vertexCount() && mesh->indexCount())) {
            LOG_WARN("Can't use mesh without position or indices as collision mesh");
            return;
        }
        this->mesh = mesh;
        indexVertexArray.reset(new btTriangleIndexVertexArray(
            mesh->indexCount() / 3,
            (int32_t*)mesh->getPermanentIndexData(),
            sizeof(uint32_t) * 3,
            mesh->vertexCount(),
            (btScalar*)mesh->getPermanentVertexData(),
            sizeof(float) * 3
        ));
        bt_mesh_shape.reset(new btBvhTriangleMeshShape(
            indexVertexArray.get(), true
        ));
        bt_shape = bt_mesh_shape;
    }

    std::shared_ptr<Mesh> mesh;

    std::shared_ptr<btEmptyShape> empty;
    std::shared_ptr<btBvhTriangleMeshShape> bt_mesh_shape;
    std::shared_ptr<btTriangleIndexVertexArray> indexVertexArray;
};

REG_TYPE(EmptyShape_);
REG_TYPE(SphereShape_);
REG_TYPE(BoxShape_);
REG_TYPE(CapsuleShape_);
REG_TYPE(MeshShape_);

#endif
