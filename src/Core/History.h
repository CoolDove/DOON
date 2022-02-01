#pragma once
#include <stack>

class Command
{
public:
    virtual void on_undo() {};// this is for undo to work
    virtual void on_redo() {};// this is for redo to work
};

class HistorySys
{
public:
    void push(Command* p_cmd, bool _do_action = false);
    void undo();
    void redo();
private:
    void clear_redo_stack();
private:
    std::stack<Command*> history_;
    std::stack<Command*> redo_stack_;
};
