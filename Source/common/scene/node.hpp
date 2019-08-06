#ifndef SCENE_OBJECT_HPP
#define SCENE_OBJECT_HPP

#include <rttr/type>
#include <rttr/registration>

#include <set>
#include <memory>

#include "../../common/util/log.hpp"
#include "../../common/util/static_run.h"

#include "../transform.hpp"

#include "../attributes/attribute.hpp"
#include "../scene/scene_controller.hpp"

enum OBJECT_TYPE {
    OBJECT_NORMAL,
    OBJECT_INSTANCE
};
enum OBJECT_FLAGS {
    OBJECT_FLAG_NONE = 0,
    OBJECT_FLAG_TRANSIENT = 1
};

class ktObjectInstance;
class GameScene;
class ktNode {
public:
    ktNode();
    virtual ~ktNode();

    void                                setEnabled(bool v);
    bool                                isEnabled() const;
    void                                setFlags(OBJECT_FLAGS f) { _flags = f; }
    OBJECT_FLAGS                        getFlags() const { return _flags; }
    virtual OBJECT_TYPE                 getType() const { return OBJECT_NORMAL; }

    void                                copy(ktNode* other, OBJECT_FLAGS f = OBJECT_FLAG_NONE);

    void                                setName(const std::string& name);
    const std::string&                  getName() const;

    ktNode*                         getRoot();
    ktNode*                         getParent();

    TransformNode*                      getTransform();

    ktNode*                         createChild(OBJECT_FLAGS f = OBJECT_FLAG_NONE);
    ktObjectInstance*                   createInstance(std::shared_ptr<GameScene> scn);
    size_t                              childCount();
    ktNode*                         getChild(size_t i);
    ktNode*                         getChild(const std::string& name);
    ktNode*                         findObject(const std::string& name);
    void                                getAllObjects(std::vector<ktNode*>& result);
    void                                takeOwnership(ktNode* o);
    void                                remove(bool keep_children = false);
    void                                duplicate();

    std::shared_ptr<Attribute>          find(rttr::type component_type);
    std::shared_ptr<Attribute>          get(rttr::type component_type);
    template<typename COMPONENT_T>
    std::shared_ptr<COMPONENT_T>        get();
    template<typename COMPONENT_T>
    std::shared_ptr<COMPONENT_T>        find();
    size_t                              componentCount();
    std::shared_ptr<Attribute>          getById(size_t id);
    void                                deleteComponentById(size_t id);
    void                                deleteAllComponents();

    void                                refreshAabb();
    void                                setAabb(const gfxm::aabb& box);
    const gfxm::aabb&                   getAabb() const;

    virtual void                        onGui();
    void                                onGizmo(GuiViewport& vp);
    bool                                serializeComponents(std::ostream& out);

    void                                write(out_stream& out);
    void                                read(in_stream& in);
    bool                                write(const std::string& fname);
    bool                                read(const std::string& fname);

    virtual void                        _readdComponent(Attribute* attrib);
protected:
    std::shared_ptr<Attribute>          createComponent(rttr::type t);
    virtual void                        _registerComponent(Attribute* attrib);
    virtual void                        _unregisterComponent(Attribute* attrib);

    bool _enabled = true;
    OBJECT_FLAGS _flags = OBJECT_FLAG_NONE;
    std::string name = "Object";
    TransformNode transform;
    gfxm::aabb aabb = gfxm::aabb(
        gfxm::vec3(-.5f, -.5f, -.5f),
        gfxm::vec3(.5f, .5f, .5f)
    );
    ktNode* parent = 0;
    std::set<ktNode*> children;
    std::map<rttr::type, std::shared_ptr<Attribute>> components;
};

template<typename COMPONENT_T>
std::shared_ptr<COMPONENT_T> ktNode::get() {
    return std::dynamic_pointer_cast<COMPONENT_T>(get(rttr::type::get<COMPONENT_T>()));
}
template<typename COMPONENT_T>
std::shared_ptr<COMPONENT_T> ktNode::find() {
    return std::dynamic_pointer_cast<COMPONENT_T>(find(rttr::type::get<COMPONENT_T>()));
}

#endif