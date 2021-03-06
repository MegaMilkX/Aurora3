#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include <rttr/type>
#include <rttr/registration>
#include "../../common/util/static_run.h"

#include "../../common/gfxm.hpp"

#include "../../common/util/data_stream.hpp"
#include "../../common/util/data_writer.hpp"

#include "../../common/util/log.hpp"

#include "../../common/lib/imgui_wrap.hpp"

#include "../debug_draw.hpp"
#include "../gui_viewport.hpp"

#include "../../common/util/materialdesign_icons.hpp"

#include "attrib_type_lib.hpp"

#include "../scene_byte_stream.hpp"

class ktNode;
class GameScene;
class Attribute {
    RTTR_ENABLE()

    friend GameScene;
    friend ktNode;
public:
    virtual ~Attribute();

    virtual rttr::type getRequiredOwnerType();

    // Basically a constructor for components
    virtual void onCreate();

    virtual void copy(Attribute* other);

    ktNode* getOwner();
    void resetAttribute();

    virtual bool buildAabb(gfxm::aabb& out);

    virtual bool requiresTransformCallback() const { return false; }

    virtual void onGui();
    virtual void onGizmo(GuiViewport& vp);
    virtual const char* getIconCode() const { return ""; }

    virtual void write(SceneWriteCtx& out);
    virtual void read(SceneReadCtx& in);
private:
    ktNode* owner = 0;
};

#define REG_ATTRIB_INL(TYPE, NAME, CATEGORY) \
    rttr::registration::class_<TYPE>(#NAME) \
        .constructor<>()( \
            rttr::policy::ctor::as_raw_ptr \
        ); \
    getAttribTypeLib().add(#CATEGORY, rttr::type::get<TYPE>());

#define REG_ATTRIB(TYPE, NAME, CATEGORY) \
STATIC_RUN(TYPE) { \
    REG_ATTRIB_INL(TYPE, NAME, CATEGORY) \
}

#endif
