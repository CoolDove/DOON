#pragma once

class Command
{
public:
    Command() {
    }

public:
    virtual void on_undo() {};// this is for undo to work
    virtual void on_redo() {};// this is for redo to work
};
