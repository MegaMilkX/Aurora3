#ifndef ANIM_BLACKBOARD_HPP
#define ANIM_BLACKBOARD_HPP

#include <map>
#include <vector>
#include <string>


class AnimBlackboard;
class AnimBlackboardParamHdl {
    AnimBlackboard* bb = 0;
    size_t index = 0;
public:
    AnimBlackboardParamHdl(AnimBlackboard* bb, size_t index)
    : bb(bb), index(index) 
    {}

    void set(float val);
    void set(int val);
    void set(bool val);
    void get_float() const;
    void get_int() const;
    void get_bool() const;
};

class AnimBlackboard {
    std::vector<float> values;
    std::map<std::string, size_t> name_hdl;
    std::map<size_t, std::string> hdl_name;

    bool exists(const char* name) {
        return name_hdl.find(name) != name_hdl.end();
    }

public:
    size_t count() const {
        return values.size();
    }

    size_t getHandle(const char* name) {
        size_t hdl = 0;
        auto it = name_hdl.find(name);
        if(it == name_hdl.end()) {
            hdl = values.size();
            name_hdl[name] = hdl;
            hdl_name[hdl] = name;
            values.emplace_back(.0f);
        } else {
            hdl = it->second;
        }
        return hdl;
    }
    const char* getName(size_t hdl) {
        auto& it = hdl_name.find(hdl);
        if(it == hdl_name.end()) {
            return 0;
        }
        return it->second.c_str();
    }

    void rename(size_t hdl, const char* name) {
        auto& it = hdl_name.find(hdl);
        if(it == hdl_name.end()) {
            return;
        }
        std::string old_name = it->second;

        hdl_name.erase(it);
        name_hdl.erase(old_name);

        hdl_name[hdl] = name;
        name_hdl[name] = hdl;
    }

    void set(size_t hdl, float val) {
        values[hdl] = val;
    }
    void set(size_t hdl, int val) {
        set(hdl, (float)val);
    }
    void set(size_t hdl, bool val) {
        if(val) {
            set(hdl, 1.0f);
        } else {
            set(hdl, .0f);
        }
    }

    float get_float(size_t hdl) {
        return values[hdl];
    }
    int get_int(size_t hdl) {
        return (int)get_float(hdl);
    }
    bool get_bool(size_t hdl) {
        return ((int)get_float(hdl)) != 0;
    }
};


#endif
