#include "Action.h"
#include <regex>
#include <DoveLog.hpp>

// TODO: give up actions with args
// TODO: rebuild action system
ActionList::ActionList()
:   curr_page_("default")
{
}

ActionList::~ActionList() {
}

bool ActionList::invoke(ActionKey _key) {
    if (key_pages_[curr_page_].find(_key) == key_pages_[curr_page_].end()) {
        return false;
    } else {
        std::string action_name = key_pages_[curr_page_][_key];

        if (action_map_.find(action_name) == action_map_.end()) {
            return false;
        } else {
            Action action = action_map_[action_name];
            action();
            return true; 
        }
    }
}

Action ActionList::get_action(ActionKey _key) {
    std::string action_name = key_pages_[curr_page_][_key];
    return action_map_[action_name];
}

void ActionList::register_key(const std::string& _page, ActionKey _key, const std::string& _action_name) {
    if (key_pages_.find(_page) == key_pages_.end()) {
        ActionNameMap map;
        map[_key] = _action_name;
        key_pages_[_page] = map;
    } else {
        key_pages_[_page][_key] = _action_name;
    }
}

void ActionList::register_action(const std::string& _name, Action _action) {
    action_map_[_name] = _action;
}

void ActionList::switch_page(const std::string& _name) {
    if (key_pages_.find(_name) != key_pages_.end())
        curr_page_ = _name;
}


ActionKey ActionList::parse_actionkey(const std::string& str, bool* _good) {
    using namespace Dove;
    *_good = true;
    ActionKey key;
    key.key = KeyCode::None;
    key.mod = ModKey::None;

    uint32_t ind = 0;

    bool slash_need = false;
    if (str[ind++] == '<') {
        while (ind < str.size()) {
            if (slash_need) {
                if (str[ind] == '-') {
                    slash_need = false;
                    ind++;
                } else {
                    DLOG_ERROR("failed to parse action key: %s", str.c_str());
                    *_good = false;
                    return key;
                }
            }
            if (str[ind] == 'S') {
                key.mod |= ModKey::Shift;
                slash_need = true;
                ind++;
            } else if (str[ind] == 'C') {
                key.mod |= ModKey::Ctrl;
                slash_need = true;
                ind++;
            } else if (str[ind] == 'M') {
                key.mod |= ModKey::Alt;
                slash_need = true;
                ind++;
            } else if ('a' <= str[ind] <= 'z') {
                key.key = KeyCode(10u + uint32_t(str[ind] - 'a'));
                slash_need = true;
                ind++;
                if (str[ind] == '>') return key;
                else {
                    DLOG_ERROR("failed to parse action key: %s", str.c_str());
                    *_good = false;
                    return key;
                }
            } else {
                DLOG_ERROR("failed to parse action key: %s", str.c_str());
                *_good = false;
                return key;
            }
        }
    } else {
        DLOG_ERROR("failed to parse action key: %s", str.c_str());
        *_good = false;
    }
    
    return key;
}

bool ActionList::config_keymap(const SettingPair* settings, const char* map_name) {
    bool good = true;

    std::unordered_map<ActionKey, std::string> map;// key to command

    for (auto ite = settings->cbegin(); ite != settings->end(); ite++) {
        bool parse_success = false;
        ActionKey akey = parse_actionkey(ite->first, &parse_success);
        if (parse_success) {
            map[akey] = ite->second;
        } else {
            good = false;
        }
    }

    if (good) {
        if (key_pages_.find(map_name) != key_pages_.end())
            key_pages_.clear();
        for (auto ite = map.cbegin(); ite != map.cend(); ite++)
            register_key(map_name, ite->first, ite->second);
    } else {
        DLOG_ERROR("failed to initialize keymap");
    }
    return good;
}
