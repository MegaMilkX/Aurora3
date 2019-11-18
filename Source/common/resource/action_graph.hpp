#ifndef ACTION_GRAPH_HPP
#define ACTION_GRAPH_HPP

#include "resource.h"
#include "../gfxm.hpp"

#include "../resource/animation.hpp"
#include "../resource/skeleton.hpp"

#include "../util/make_unique_string.hpp"

#include "clip_motion.hpp"
#include "blend_tree_motion.hpp"

#include "../util/anim_blackboard.hpp"

class ActionGraph;

class ActionGraphNode;
struct ActionGraphTransition {
    enum CONDITION {
        LARGER,
        LARGER_EQUAL,
        LESS,
        LESS_EQUAL,
        EQUAL,
        NOT_EQUAL
    };
    struct Condition {
        size_t param_hdl;
        std::string param_name;
        CONDITION type;
        float ref_value;
    };

    float blendTime = 0.1f;
    ActionGraphNode* from = 0;
    ActionGraphNode* to = 0;
    std::vector<Condition> conditions;
};

class ActionGraphNode {
    std::string name = "Action";
    gfxm::vec2 editor_pos;
    std::set<ActionGraphTransition*> out_transitions;
public:
    std::shared_ptr<Motion> motion;

    ActionGraphNode();

    void setSkeleton(std::shared_ptr<Skeleton> skel) {
        motion->setSkeleton(skel);
    }

    const std::string& getName() const { return name; }
    void               setName(const std::string& value) { name = value; }
    const gfxm::vec2&  getEditorPos() const { return editor_pos; }
    void               setEditorPos(const gfxm::vec2& value) { editor_pos = value; }

    std::set<ActionGraphTransition*>& getTransitions() {
        return out_transitions;
    }

    void               update(
        float dt, 
        std::vector<AnimSample>& samples,
        float weight
    );

    void               write(out_stream& out);
    void               read(in_stream& in);
};

class ActionGraphLayer {
public:
};

// ==========================

class ActionGraph : public Resource {
    RTTR_ENABLE(Resource)

    std::vector<ActionGraphTransition*> transitions;
    std::vector<ActionGraphNode*> actions;
    size_t entry_action = 0;
    size_t current_action = 0;
    float trans_weight = 1.0f;
    float trans_speed = 0.1f;
    std::vector<AnimSample> trans_samples;
    AnimBlackboard* blackboard = 0;

    void pickEntryAction();

public:
    const char* getWriteExtension() const override { return "action_graph"; }

    void setSkeleton(std::shared_ptr<Skeleton> skeleton) {
        for(auto a : actions) {
            a->setSkeleton(skeleton);
        }
    }
    void setBlackboard(AnimBlackboard* bb) {
        blackboard = bb;
        for(auto& t : transitions) {
            for(auto& c : t->conditions) {
                c.param_hdl = blackboard->getHandle(c.param_name.c_str());
            }
        }
    }

    ActionGraphNode* createAction(const std::string& name = "Action");
    void             renameAction(ActionGraphNode* action, const std::string& name);
    ActionGraphTransition* createTransition(const std::string& from, const std::string& to);

    ActionGraphNode* findAction(const std::string& name);
    int32_t          findActionId(const std::string& name);

    void            deleteAction(ActionGraphNode* action);
    void            deleteTransition(ActionGraphTransition* transition);

    const std::vector<ActionGraphNode*>&       getActions() const;
    const std::vector<ActionGraphTransition*>& getTransitions() const;

    ActionGraphNode*                           getAction(size_t i);

    size_t                                     getEntryActionId();
    ActionGraphNode*                           getEntryAction();
    void                                       setEntryAction(const std::string& name);

    void                                       update(
        float dt, 
        std::vector<AnimSample>& samples
    );

    void serialize(out_stream& out) override;
    bool deserialize(in_stream& in, size_t sz) override;
};
STATIC_RUN(ActionGraph) {
    rttr::registration::class_<ActionGraph>("ActionGraph")
        .constructor<>()(
            rttr::policy::ctor::as_raw_ptr
        );
}

#endif
