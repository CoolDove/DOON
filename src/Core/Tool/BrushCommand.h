#pragma once
#include <Core/History.h>
#include <DoveLog.hpp>

class BrushCommand : public Command
{
public:
    BrushCommand(int testid) {
        testid_ = testid;
    }
public:
    int testid_;
    void on_undo() override {
        DLOG_DEBUG("undo");
    }

    void on_redo() override {
        DLOG_DEBUG("redo");
    }
};
