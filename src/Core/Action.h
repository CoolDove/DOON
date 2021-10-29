#pragma once

#include <string>
#include <unordered_map>
#include <Base/Keys.h>

enum class ActionArgType : uint32_t {
    INT, 
    FLOAT,
    STRING
};

struct ActionArgInfo {
    uint32_t count;
    std::vector<ActionArgType> types;
};

struct ActionKey {
    Dove::KeyCode key;
    Dove::ModKey  mod;
};

inline bool operator==(const ActionKey& _lhs, ActionKey _rhs) {
    return (uint32_t)_lhs.key == (uint32_t)_rhs.key&&
           (uint32_t)_lhs.mod == (uint32_t)_rhs.mod;
};

namespace std
{
template<>
struct hash<ActionKey> {
    size_t operator() (const ActionKey& _k) const noexcept
    {
        return (size_t)_k.key | ((size_t)_k.mod << 32);
    }
};
}

using ActionFunction = void(*)(ActionArgInfo, ...);
struct Action {
    ActionArgInfo  arg;
    ActionFunction action;
};

// NOTE:
// input system reads the user input, composite them to a ActionKey struct, 
// use action
class ActionList {
private:
struct ActionCall {
    // this is the parsing result from a action_command_string
    std::string              name;
    std::vector<std::string> args;
};
using ActionCallMap = std::unordered_map<ActionKey, ActionCall>;
using ActionMap     = std::unordered_map<std::string, Action>;
public:
    ActionList();
    ~ActionList();

    Action get_action(ActionKey _key);

    void register_action(const std::string& _page, const std::string& _name, Action _action);
    void switch_page(const std::string& _page_name);

// NOTE: register_key should be controlled by only parsing module, so it will be private after that
    void register_key(const std::string& _page, ActionKey _key, const std::string& _command);

    void load_config();
private:
    std::string curr_page_;
    std::unordered_map<std::string, ActionCallMap> call_pages_;
    std::unordered_map<std::string, ActionMap>     action_pages_;
};