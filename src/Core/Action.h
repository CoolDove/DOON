#pragma once

#include <stdint.h>
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
inline bool operator==(const ActionArgInfo& _lhs, ActionArgInfo _rhs) {
    return _lhs.count == _rhs.count &&
           _lhs.types == _rhs.types;
};

struct ActionKey {
    Dove::KeyCode key;
    Dove::ModKey  mod;
};
inline std::string to_string(const ActionKey &_key) {
    using namespace Dove;
    std::string output = "<";
    // bool moded = false;
    uint32_t ukey = (uint32_t)_key.key;
    uint32_t umod = (uint32_t)_key.mod;
    if (umod & (uint32_t)ModKey::Alt) output += "Alt-";
    if (umod & (uint32_t)ModKey::Ctrl) output += "Ctrl-";
    if (umod & (uint32_t)ModKey::Shift) output += "Shift-";
    if (umod & (uint32_t)ModKey::Space) output += "Space-";

    if (ukey >= 10u && ukey) {
        output += (char)('A' + ukey - 10u);
    }      
    output += '>';
    return output;
}  

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

using Action = void(*)();
// NOTE:
// input system reads the user input, composite them to a ActionKey struct, 
// use action
class ActionList {
private:
using ActionNameMap = std::unordered_map<ActionKey, std::string>;
using ActionMap     = std::unordered_map<std::string, Action>;
public:
    ActionList();
    ~ActionList();

    Action get_action(ActionKey _key);
    bool invoke(ActionKey _key);

    void register_action(const std::string& _page, const std::string& _name, Action _action);
    void switch_page(const std::string& _page_name);

// NOTE: register_key should be controlled by parser, so it will be private later
    void register_key(const std::string& _page, ActionKey _key, const std::string& _command);

    void load_config();

public:
    std::string curr_page_;
    std::unordered_map<std::string, ActionNameMap> call_pages_;
    std::unordered_map<std::string, ActionMap>     action_pages_;
};
