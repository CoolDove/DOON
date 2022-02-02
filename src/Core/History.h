#pragma once
#include <stack>
#include "Command.h"

class HistorySys
{
public:
    void push(Command* p_cmd, bool _do_action = false);
    void undo();
    void redo();
public:
    int count_redo_stack() const { return redo_stack_.size(); };
    int count_history() const { return history_.size(); };
private:
    void clear_redo_stack();
private:
    std::stack<Command*> history_;
    std::stack<Command*> redo_stack_;
};
