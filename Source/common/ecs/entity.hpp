#ifndef ECS_ENTITY_HPP
#define ECS_ENTITY_HPP

#include <stdint.h>
#include <map>
#include <memory>

#include "attribute.hpp"


static const entity_id NULL_ENTITY = -1;


inline ecsAttribBase* allocAttrib(attrib_id id) {
    ecsAttribBase* ptr = 0;
    auto inf = getEcsAttribTypeLib().get_info(id);
    if(!inf) {
        LOG_WARN("Attribute info for id " << id << " doesn't exist");
        assert(false);
        return 0;
    }
    ptr = inf->constructor();
    if(!ptr) {
        LOG_WARN("Constructor for attrib " << inf->name << " failed");
        assert(false);
        return 0;
    }
    return ptr;
}

class ecsWorld;
class ecsEntity {
    friend ecsWorld;

    entity_id parent_uid = NULL_ENTITY;
    entity_id entity_uid;
    uint64_t attrib_bits;
    std::map<uint8_t, std::shared_ptr<ecsAttribBase>> attribs;
    uint64_t bitmaskInheritedAttribs = 0;

    ecsAttribBase* allocAttribOwned(attrib_id id) {
        auto ptr = allocAttrib(id);
        ptr->entity = entity_uid;
        return ptr;
    }

public:
    template<typename T>
    T* getAttrib() {
        return dynamic_cast<T*>(getAttrib(T::get_id_static()));
    }

    ecsAttribBase* getAttrib(attrib_id id) {
        ecsAttribBase* ptr = findAttrib(id); // TODO: ???
        ptr = allocAttribOwned(id);
        attribs[(uint8_t)id].reset(ptr);
        setBit(id);
        return ptr;
    }
    template<typename T>
    T* setAttrib(const T& value) {
        T* ptr = findAttrib<T>();
        if(!ptr) {
            ptr = (T*)allocAttribOwned(T::get_id_static());
            if(!ptr) {
                return 0;
            }
            attribs[(uint8_t)T::get_id_static()].reset(ptr);
            setBit(T::get_id_static());
        }
        *ptr = value;
        return ptr;
    }

    void removeAttrib(attrib_id id) {
        attribs.erase((uint8_t)id);
        clearBit(id);
    }

    template<typename T>
    T* findAttrib() {
        return dynamic_cast<T*>(findAttrib(T::get_id_static()));
    }

    ecsAttribBase* findAttrib(attrib_id id) {
        uint64_t mask = 1 << id;
        if(attrib_bits & mask) {
            return attribs[id].get();
        }
        return 0;
    }

    ecsAttribBase* getAttribPtr(uint8_t attrib_id) {
        auto it = attribs.find(attrib_id);
        if(it == attribs.end()) {
            return 0;
        }
        return it->second.get();
    }

    template<typename T>
    bool updateAttrib(const T& value) {
        auto it = attribs.find(T::get_id_static());
        if(it == attribs.end()) {
            return false;
        }
        *(T*)(it->second.get()) = (value);        
        return true;
    }

    const uint64_t& getAttribBits() const {
        return attrib_bits;
    }
    void setBit(attrib_id attrib) {
        attrib_bits |= (1 << attrib);
    }
    void clearBit(attrib_id attrib) {
        attrib_bits &= ~(1 << attrib);
    }

};

#endif
