#include "game_object.hpp"
#include "../behavior/behavior.hpp"

#include "game_scene.hpp"

#define MINIZ_HEADER_FILE_ONLY
#include "../../common/lib/miniz.c"

GameObject::GameObject() {

}
GameObject::~GameObject() {
    clearBehavior();
    // Tell parent transform that this one doesn't exist anymore
    transform.setParent(0);
    for(auto c : children) {
        getScene()->_unregObject(c.get());
        getScene()->getEventMgr().postObjectRemoved(c.get());
    }
    deleteAllComponents();
}

void GameObject::setName(const std::string& name) { 
    this->name = name; 
}
const std::string& GameObject::getName() const { 
    return name; 
}

GameScene* GameObject::getScene() {
    return scene;
}
GameObject* GameObject::getRoot() {
    if(!parent) {
        return this;
    } else {
        return parent->getRoot();
    }
}
GameObject* GameObject::getParent() {
    return parent;
}

TransformNode* GameObject::getTransform() {
    return &transform;
}

void GameObject::copy(GameObject* other) {
    setName(other->getName());
    getTransform()->setPosition(other->getTransform()->getPosition());
    getTransform()->setRotation(other->getTransform()->getRotation());
    getTransform()->setScale(other->getTransform()->getScale());
}
void GameObject::copyRecursive(GameObject* o) {
    for(size_t i = 0; i < o->childCount(); ++i) {
        auto n = createChild(o->getChild(i)->get_type());
        n->copyRecursive(o->getChild(i).get());
    }
    copy(o);
}
void GameObject::copyComponents(GameObject* other) {
    for(size_t i = 0; i < other->componentCount(); ++i) {
        auto c = other->getById(i);
        auto new_c = get(c->get_type());
        new_c->copy(c.get());
    }
}
void GameObject::copyComponentsRecursive(GameObject* other) {
    for(size_t i = 0; i < other->childCount(); ++i) {
        auto child = getChild(other->getChild(i)->getName());
        if(!child) continue;
        child->copyComponentsRecursive(other->getChild(i).get());
    }
    copyComponents(other);

    auto bhvr = other->getBehavior();
    if(bhvr) {
        dstream strm;
        bhvr->serialize(strm);
        setBehavior(bhvr->get_type());
        bhvr = getBehavior();
        if(bhvr) {
            bhvr->deserialize(strm);
        }
    }
}
void GameObject::copyEmptyTree(GameObject* other) {
    for(size_t i = 0; i < other->childCount(); ++i) {
        auto n = createChild(other->getChild(i)->get_type());
        n->copyEmptyTree(other->getChild(i).get());
    }
    setName(other->getName());
}
void GameObject::cloneExistingTree(GameObject* other) {
    for(size_t i = 0; i < other->childCount(); ++i) {
        auto child = getChild(other->getChild(i)->getName());
        if(!child) continue;
        child->cloneExistingTree(other->getChild(i).get());
    }
    copy(other);
}

void GameObject::setBehavior(rttr::type t) {
    if(!scene) return;
    scene->createBehavior(this, t);
}
Behavior* GameObject::getBehavior() {
    if(!scene) return 0;
    return scene->findBehavior(this);
}
void GameObject::clearBehavior() {
    if(!scene) return;
    scene->eraseBehavior(this);
}

std::shared_ptr<GameObject> GameObject::createChild() {
    return createChild(rttr::type::get<GameObject>());
}
std::shared_ptr<GameObject> GameObject::createChild(rttr::type t) {
    if(!t.is_valid()) {
        LOG_WARN(t.get_name().to_string() << " - not a valid type");
        return std::shared_ptr<GameObject>();
    }
    rttr::variant v = t.create();
    if(!v.get_type().is_pointer()) {
        LOG_WARN(t.get_name().to_string() << " - rttr constructor policy must be pointer");
        return std::shared_ptr<GameObject>();
    }
    GameObject* c = v.get_value<GameObject*>();
    if(!c) {
        LOG_WARN("Failed to create component " << t.get_name().to_string());
        return std::shared_ptr<GameObject>();
    }
    auto o = std::shared_ptr<GameObject>(c);
    o->scene = scene;
    o->parent = this;
    o->getTransform()->setParent(&transform);
    children.insert(o);
    scene->_regObject(o.get());
    scene->getEventMgr().postObjectCreated(o.get());
    return o;
}
std::shared_ptr<GameObject> GameObject::createClone(GameObject* o) {
    auto n = createChild(o->get_type());
    n->copyRecursive(o);
    n->copyComponentsRecursive(o);
    return n;
}

size_t GameObject::childCount() {
    return children.size();
}
std::shared_ptr<GameObject> GameObject::getChild(size_t i) {
    auto it = children.begin();
    std::advance(it, i);
    return (*it);
}
std::shared_ptr<GameObject> GameObject::getChild(const std::string& name) {
    for(auto o : children) {
        if(o->getName() == name) {
            return o;
        }
    }
    return std::shared_ptr<GameObject>();
}
GameObject* GameObject::findObject(const std::string& name) {
    for(auto o : children) {
        if(o->getName() == name) return o.get();
        GameObject* r = o->findObject(name);
        if(r) return r;
    }
    return 0;
}
void GameObject::getAllObjects(std::vector<GameObject*>& result) {
    for(auto o : children) {
        o->getAllObjects(result);
    }
    result.emplace_back(this);
}
void GameObject::removeChild(GameObject* o) {
    for(auto it = children.begin(); it != children.end(); ++it) {
        if((*it).get() == o) {
            for(size_t i = 0; i < (*it)->childCount(); ++i) {
                auto n = createChild((*it)->getChild(i)->get_type());
                n->copyRecursive((*it)->getChild(i).get());
                n->copyComponentsRecursive((*it)->getChild(i).get());
            }
        }
    }
    removeChildRecursive(o);
}
void GameObject::removeChildRecursive(GameObject* o) {
    for(auto it = children.begin(); it != children.end(); ++it) {
        if((*it).get() == o) {
            getScene()->_unregObject(o);
            getScene()->getEventMgr().postObjectRemoved(o);
            children.erase(it);
            break;
        }
    }
}

std::shared_ptr<Attribute> GameObject::find(rttr::type component_type) {
    if(components.count(component_type) == 0) {
        return std::shared_ptr<Attribute>();
    }
    return components[component_type];
}
std::shared_ptr<Attribute> GameObject::get(rttr::type component_type) {
    auto c = find(component_type);
    if(!c) {
        return createComponent(component_type);
    }
    return c;
}
size_t GameObject::componentCount() {
    return components.size();
}
std::shared_ptr<Attribute> GameObject::getById(size_t id) {
    auto it = components.begin();
    std::advance(it, id);
    return it->second;
}
void GameObject::deleteComponentById(size_t id) {
    auto it = components.begin();
    std::advance(it, id);
    getScene()->getEventMgr().postComponentRemoved(it->second.get());
    getScene()->_unregisterComponent(it->second.get());
    components.erase(it);
}
void GameObject::deleteAllComponents() {
    for(auto& kv : components) {
        getScene()->getEventMgr().postComponentRemoved(kv.second.get());
        getScene()->_unregisterComponent(kv.second.get());
    }
    components.clear();
}

void GameObject::refreshAabb() {
    for(size_t i = 0; i < childCount(); ++i) {
        getChild(i)->refreshAabb();
    }

    aabb = gfxm::aabb(
        transform.getWorldPosition(),
        transform.getWorldPosition()
    );
    bool has_aabb = false;
    for(auto& kv : components) {
        auto oc = kv.second;
        gfxm::aabb box;
        if(oc->buildAabb(box)) {
            if(!has_aabb) {
                aabb = box;
                has_aabb = true;
            } else {
                gfxm::expand_aabb(aabb, box.from);
                gfxm::expand_aabb(aabb, box.to);
            }
        }
    }
    if(has_aabb) {
        gfxm::vec3 cube_pts[8] = {
            { aabb.from.x, aabb.from.y, aabb.from.z },
            { aabb.to.x, aabb.from.y, aabb.from.z },
            { aabb.to.x, aabb.to.y, aabb.from.z },
            { aabb.from.x, aabb.to.y, aabb.from.z },
            { aabb.from.x, aabb.from.y, aabb.to.z },
            { aabb.to.x, aabb.from.y, aabb.to.z },
            { aabb.to.x, aabb.to.y, aabb.to.z },
            { aabb.from.x, aabb.to.y, aabb.to.z }
        };
        for(size_t i = 0; i < 8; ++i) {
            cube_pts[i] = transform.getWorldTransform() * gfxm::vec4(cube_pts[i], 1.0f);
        }
        aabb = gfxm::aabb(
            cube_pts[0],
            cube_pts[0]
        );
        for(size_t i = 0; i < 8; ++i) {
            gfxm::expand_aabb(aabb, cube_pts[i]);
        }
    }

    if(!has_aabb && childCount()) {
        auto& c = getChild(0);
        aabb = gfxm::aabb(
            c->getAabb().from,
            c->getAabb().to
        );
    }

    for(size_t i = 0; i < childCount(); ++i) {
        auto& child_box = getChild(i)->getAabb();
        // TODO: Ignore empty aabbs
        gfxm::expand_aabb(aabb, child_box.from);
        gfxm::expand_aabb(aabb, child_box.to);
    }
}
void GameObject::setAabb(const gfxm::aabb& box) {
    aabb = box;
}
const gfxm::aabb& GameObject::getAabb() const {
    return aabb;
}

static bool zipAdd(void* data, size_t sz, mz_zip_archive& archive, const std::string& name) {    
    if(!mz_zip_writer_add_mem(
        &archive, 
        name.c_str(), 
        data, 
        sz, 
        0
    )){
        LOG_ERR("Failed to mz_zip_writer_add_mem() ");
    }
    return true;
}

static bool zipAddFromStream(in_stream& in, mz_zip_archive& archive, const std::string& name) {
    std::vector<char> buf;
    buf.resize(in.bytes_available());
    in.read((char*)buf.data(), in.bytes_available());
    
    if(!mz_zip_writer_add_mem(
        &archive, 
        name.c_str(), 
        buf.data(), 
        buf.size(), 
        0
    )){
        LOG_ERR("Failed to mz_zip_writer_add_mem() ");
    }
    return true;
}

void GameObject::serialize(std::ostream& out) {
    
}

void GameObject::deserialize(std::istream& in, size_t sz) {
    
}

void GameObject::onGui() {
    static TransformNode* last_transform = 0;
    static int t_sync = -1;
    static gfxm::vec3 euler;

    if(last_transform != getTransform() || t_sync != getTransform()->getSyncId()) {
        last_transform = getTransform();
        t_sync = getTransform()->getSyncId();
        euler = getTransform()->getEulerAngles();
    }

    auto t = getTransform()->getPosition();
    auto r = getTransform()->getEulerAngles();
    auto s = getTransform()->getScale();

    char buf[256];
    memset(buf, 0, 256);
    memcpy(buf, getName().c_str(), getName().size());
    if(ImGui::InputText("Name", buf, 256)) {
        setName(buf);
    }
    ImGui::Separator();
    if(ImGui::DragFloat3("Translation", (float*)&t, 0.001f)) {
        getTransform()->setPosition(t);
        getRoot()->refreshAabb();
        t_sync = getTransform()->getSyncId();
    }
    if(ImGui::DragFloat3("Rotation", (float*)&euler, 0.001f)) {
        getTransform()->setRotation(euler);
        getRoot()->refreshAabb();
        t_sync = getTransform()->getSyncId();
    }
    if(ImGui::DragFloat3("Scale", (float*)&s, 0.001f)) {
        getTransform()->setScale(s);
        getRoot()->refreshAabb();
        t_sync = getTransform()->getSyncId();
    }
}

// ==== Private ====================================

std::shared_ptr<Attribute> GameObject::createComponent(rttr::type t) {
    if(!t.is_valid()) {
        LOG_WARN(t.get_name().to_string() << " - not a valid type");
        return std::shared_ptr<Attribute>();
    }
    rttr::variant v = t.create();
    if(!v.get_type().is_pointer()) {
        LOG_WARN(t.get_name().to_string() << " - rttr constructor policy must be pointer");
        return std::shared_ptr<Attribute>();
    }
    Attribute* c = v.get_value<Attribute*>();
    if(!c) {
        LOG_WARN("Failed to create component " << t.get_name().to_string());
        return std::shared_ptr<Attribute>();
    }
    c->owner = this;
    auto ptr = std::shared_ptr<Attribute>(c);

    if(!get_type().is_derived_from(ptr->getRequiredOwnerType()) 
        && get_type() != ptr->getRequiredOwnerType()) {
        LOG_WARN("Can't create component of type " << t.get_name().to_string() << " for object of type " << get_type().get_name().to_string());
        return std::shared_ptr<Attribute>();
    }

    components[t] = ptr;
    ptr->onCreate();
    getScene()->_registerComponent(c);
    getScene()->getEventMgr().postComponentCreated(c);
    return ptr;
}

bool GameObject::serializeComponents(std::ostream& out) {
    mz_zip_archive archive;
    memset(&archive, 0, sizeof(archive));
    if(!mz_zip_writer_init_heap(&archive, 0, 65537)) {
        LOG("Failed to create archive file in memory");
        return false;
    }

    for(auto& kv : components) {
        auto& c = kv.second;
        dstream strm;
        if(c->serialize(strm)) {
            strm.jump(0);
            zipAddFromStream(strm, archive, c->get_type().get_name().to_string());
        }
    }

    void* archbuf = 0;
    size_t sz = 0;
    mz_zip_writer_finalize_heap_archive(&archive, &archbuf, &sz);
    std::vector<char> buf = std::vector<char>((char*)archbuf, (char*)archbuf + sz);
    mz_zip_writer_end(&archive);

    out.write(buf.data(), buf.size());
    return true;
}

void GameObject::write(out_stream& out) {
    GameObject* obj = this;

    std::map<GameObject*, uint64_t> oid_map;
    std::vector<GameObject*> objects;
    std::map<rttr::type, std::vector<Attribute*>> attribs;

    std::stack<GameObject*> stack;
    GameObject* current = obj;
    while(current != 0 || (!stack.empty())) {
        if(current == 0) {
            current = stack.top();
            stack.pop();
        }
        for(size_t i = 0; i < current->childCount(); ++i) {
            stack.push(current->getChild(i).get());
        }

        uint64_t id = oid_map.size();
        oid_map[current] = id;
        objects.emplace_back(current);
        for(size_t i = 0; i < current->componentCount(); ++i) {
            auto attrib = current->getById(i);
            attribs[attrib->get_type()].emplace_back(attrib.get());
        }

        current = 0;
    }

    DataWriter dw(&out);
    dw.write<uint64_t>(objects.size());
    for(size_t i = 0; i < objects.size(); ++i) {
        auto o = objects[i];
        if(o->getParent()) {
            dw.write<uint64_t>(oid_map[o]);
        } else {
            dw.write<uint64_t>(-1);
        }
        dw.write(o->getName());
        dw.write(o->getTransform()->getPosition());
        dw.write(o->getTransform()->getRotation());
        dw.write(o->getTransform()->getScale());
    }

    dw.write<uint32_t>(attribs.size());
    for(auto kv : attribs) {
        rttr::type t = kv.first;
        auto& vec = kv.second;
        dw.write(t.get_name().to_string());
        dw.write<uint64_t>(vec.size());
        for(size_t i = 0; i < vec.size(); ++i) {
            auto a = vec[i];
            dw.write<uint64_t>(oid_map[a->getOwner()]);
            
            dstream sdata;
            a->serialize(sdata);
            dw.write<uint64_t>(sdata.size()); // bytes written
            dw.write(sdata.getBuffer());
        }
    }
}
void GameObject::read(in_stream& in) {
    std::vector<GameObject*> objects;
    objects.emplace_back(this);
    
    DataReader dr(&in);
    uint64_t object_count = dr.read<uint64_t>();
    for(uint64_t i = 1; i < object_count; ++i) {
        objects.emplace_back(new GameObject());
    }

    for(uint64_t i = 1; i < object_count; ++i) {
        uint64_t p = dr.read<uint64_t>();
        std::string name = dr.readStr();
        gfxm::vec3 t = dr.read<gfxm::vec3>();
        gfxm::quat r = dr.read<gfxm::quat>();
        gfxm::vec3 s = dr.read<gfxm::vec3>();

        objects[i]->parent = objects[p];
        objects[i]->scene = this->scene;
        objects[i]->getTransform()->setPosition(t);
        objects[i]->getTransform()->setRotation(r);
        objects[i]->getTransform()->setScale(s);
        objects[p]->children.insert(std::shared_ptr<GameObject>(objects[i]));
    }

    // Send events
    for(uint64_t i = 1; i < object_count; ++i) {
        auto o = objects[i];
        scene->_regObject(o);
        scene->getEventMgr().postObjectCreated(o);
    }
}