#ifndef RESOURCE_H
#define RESOURCE_H

#include <fstream>

#include "data_source.h"
#include <rttr/type>
#include <rttr/registration>
#include <rttr/registration_friend>
#include "../util/static_run.h"
#include "../lib/imgui_wrap.hpp"

class Resource {
    RTTR_ENABLE()
public:
    enum STORAGE
    {
        GLOBAL,
        LOCAL
    };

    virtual ~Resource() {}
    const std::string& Name() const { return name; }
    void Name(const std::string& name) { this->name = name; }
    STORAGE Storage() const { return storage; }
    void Storage(STORAGE storage) { this->storage = storage; }

    virtual void serialize(std::ostream& out) {}
    virtual bool deserialize(std::istream& in, size_t sz) { return false; }

    void write_to_file(const std::string& filename) {
        std::ofstream f(filename, std::ios::binary);
        if(f.is_open()) {
            serialize(f);
        }
    }

    virtual void _editorGui() {}
private:
    std::string name;
    STORAGE storage;
};

#endif
