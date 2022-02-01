#include "History.h"

using namespace std;

void HistorySys::push(Command* p_cmd, bool _do_action) {
    history_.push(p_cmd);
    if (_do_action) p_cmd->on_redo();
    // redo_stack_.clear();
    clear_redo_stack();
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
    for (int i = 0; i < redo_stack_.size(); i++) {
        Command* cmd = redo_stack_.top();
        redo_stack_.pop();
        delete cmd;
    }
}


