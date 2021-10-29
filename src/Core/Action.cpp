#include "Action.h"

// @test:
void func(ActionArgInfo _info, ...) {
    ActionArgInfo info = _info;

    int ct = info.count;
}

ActionList::ActionList()
:   curr_page_("def")
{
    // @Temp:build action list in code, will be replaced by .doon parsing
    using namespace Dove;
    register_key("def", ActionKey{KeyCode::A, ModKey::None}, "hello_dove");
    register_action("def", "hello_dove", {ActionArgInfo{1, std::vector<ActionArgType>{0}}, func});
}

ActionList::~ActionList() {

}

Action ActionList::get_action(ActionKey _key) {
    ActionCall call = call_pages_[curr_page_][_key];
    return action_pages_[curr_page_][call.name];
}
void ActionList::register_key(const std::string& _page, ActionKey _key, const std::string& _action_string) {
    // TODO: parse module to check if the action_string is valid
    ActionCall call;
    call.name = "hello_dove";
    call.args = std::vector<std::string>{};

    if (call_pages_.find(_page) == call_pages_.end()) {
        ActionCallMap map;
        map[_key] = call;
        call_pages_[_page] = map;
    } else {
        call_pages_[_page][_key] = call;
    }
}

void ActionList::register_action(const std::string& _page, const std::string& _name, Action _action) {
    if (action_pages_.find(_page) == action_pages_.end()) {
        ActionMap map;
        map[_name] = _action;
        action_pages_[_page] = map;
    } else {
        action_pages_[_page][_name] = _action;
    }
}

void ActionList::switch_page(const std::string& _name) {
    curr_page_ = _name;
}