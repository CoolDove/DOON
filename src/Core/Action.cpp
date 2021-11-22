#include "Action.h"
#include <regex>
#include <DoveLog.hpp>

// TODO: give up actions with args
// TODO: rebuild action system
ActionList::ActionList()
:   curr_page_("def")
{
    // @Temp: build action list in code, will be replaced by .doon parsing
    // using namespace Dove;
    // register_key("def", ActionKey{KeyCode::A, ModKey::None}, "press_a");
    // register_action("def", "press_a", press_a);    
    // register_key("def", ActionKey{KeyCode::A, ModKey::Shift}, "press_s_a");
    // register_action("def", "press_s_a", press_s_a);    
    // register_key("def", ActionKey{KeyCode::A, ModKey::Shift | ModKey::Ctrl}, "press_c_s_a");
    // register_action("def", "press_c_s_a", press_c_s_a);    
}

ActionList::~ActionList() {
}

bool ActionList::invoke(ActionKey _key) {
    if (call_pages_[curr_page_].find(_key) == call_pages_[curr_page_].end()) {
        return false;
    } else {
        std::string action_name = call_pages_[curr_page_][_key];

        if (action_pages_[curr_page_].find(action_name) == action_pages_[curr_page_].end()) {
            return false;
        } else {
            Action action = action_pages_[curr_page_][action_name];
            action();
            return true; 
        }
    }
}

Action ActionList::get_action(ActionKey _key) {
    std::string action_name = call_pages_[curr_page_][_key];
    return action_pages_[curr_page_][action_name];
}

void ActionList::register_key(const std::string& _page, ActionKey _key, const std::string& _action_name) {
    if (call_pages_.find(_page) == call_pages_.end()) {
        ActionNameMap map;
        map[_key] = _action_name;
        call_pages_[_page] = map;
    } else {
        call_pages_[_page][_key] = _action_name;
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
