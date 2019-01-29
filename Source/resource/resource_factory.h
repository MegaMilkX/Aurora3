#ifndef RESOURCE_FACTORY_H
#define RESOURCE_FACTORY_H

#include "resource.h"
#include "data_registry.h"
#include <unordered_map>
#include "../util/log.hpp"

class ResourceFactory {
public:
    typedef std::weak_ptr<Resource> res_weak_ptr_t;
    typedef std::shared_ptr<Resource> res_shared_ptr_t;
    typedef std::unordered_map<std::string, res_weak_ptr_t> resource_map_t;
    
    template<typename T>
    std::shared_ptr<T> Get(const std::string& n) {
        std::string name = n;
        for(size_t i = 0; i < name.size(); ++i) {
            name[i] = (std::tolower(name[i]));
            if(name[i] == '\\') {
                name[i] = '/';
            }
        }

        DataSourceRef dataSrc = GlobalDataRegistry().Get(name);
        if(!dataSrc) {
            LOG("Data source '" << name << "' doesn't exist.");
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> ptr;
        res_weak_ptr_t& weak = resources[name];
        if(weak.expired()) {
            auto& strm = dataSrc->open_stream();
            strm.seekg(0, std::ios::end);
            size_t sz = (size_t)strm.tellg();
            strm.seekg(0, std::ios::beg);

            ptr.reset(new T());
            if(!ptr->deserialize(strm, sz)) {
                LOG("Failed to build resource " << name);
                dataSrc->close_stream();
                return std::shared_ptr<T>();
            }
            dataSrc->close_stream();

            weak = ptr;
            ptr->Name(name);
            ptr->Storage(Resource::GLOBAL);
        } else {
            ptr = std::dynamic_pointer_cast<T>(weak.lock());
        }
        return ptr;
    }
private:
    resource_map_t resources;
};

inline ResourceFactory& GlobalResourceFactory() {
    static ResourceFactory f;
    return f;
}

template<typename T>
std::shared_ptr<T> getResource(const std::string& name) {
    return GlobalResourceFactory().Get<T>(name);
}

#endif
