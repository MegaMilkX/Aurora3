#ifndef COLLISION_TRIGGER_HPP
#define COLLISION_TRIGGER_HPP

#include "attribute.hpp"

class CollisionListener : public Attribute {
    RTTR_ENABLE(Attribute)
public:
    virtual void onEnter(ktNode* other) = 0;
    virtual void onLeave(ktNode* other) = 0;
};

#endif
