#include "History.h"
#include "DoveLog.hpp"

using namespace std;

void HistorySys::push(Command* p_cmd, bool _do_action) {
    history_.push(p_cmd);
    if (_do_action) p_cmd->on_redo();

    clear_redo_stack();

    if (history_.size() >= max_ + extension_)
        release(history_.size() - max_);
}

void HistorySys::undo() {
    if (history_.empty()) return;
    Command* cmd = history_.top();
    history_.pop();
    cmd->on_undo();
    redo_stack_.push(cmd);
}

void HistorySys::redo() {
    if (redo_stack_.empty()) return;
    Command* cmd = redo_stack_.top();
    redo_stack_.pop();
    cmd->on_redo();
    history_.push(cmd);
}

void HistorySys::clear_redo_stack() {
    int redo_count = redo_stack_.size(); 
    for (int i = 0; i < redo_count; i++) {
        Command* cmd = redo_stack_.top();
        redo_stack_.pop();
        delete cmd;
    }
}

void HistorySys::release(int count) {
    std::stack<Command*> stash;
    int left_count = history_.size() - count;
    int history_size = history_.size();

    for (int i = 0; i < history_size; i++) {
        auto* cmd = history_.top();
        history_.pop();
        
        if (i < left_count) {
            stash.push(cmd);
        } else {
            delete cmd;
        }
    }

    for (int i = 0; i < left_count; i++) {
        auto* cmd = stash.top();
        stash.pop();
        history_.push(cmd);
    }
}

