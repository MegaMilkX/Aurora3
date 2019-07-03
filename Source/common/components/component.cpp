#include "component.hpp"

#include "../scene/game_scene.hpp"

Attribute::~Attribute() {
    
}

rttr::type Attribute::getRequiredOwnerType() {
    return rttr::type::get<GameObject>();
}

void Attribute::onCreate() {}

void Attribute::copy(Attribute* other) {}

GameObject* Attribute::getOwner() { return owner; }
void Attribute::resetAttribute() {
    getOwner()->getScene()->resetAttribute(this);
}

bool Attribute::buildAabb(gfxm::aabb& out) {
    out = gfxm::aabb(
        gfxm::vec3(.0f, .0f, .0f),
        gfxm::vec3(.0f, .0f, .0f)
    );
    return false;
}

void Attribute::onGui() {

}

void Attribute::onGizmo(GuiViewport& vp) {

}

bool Attribute::serialize(out_stream& out) {
    return false;
}
bool Attribute::deserialize(in_stream& in, size_t sz) {
    return false;
}