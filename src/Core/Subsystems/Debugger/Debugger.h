#pragma once

#include <stdio.h>
#include <deque>

class Debugger
{
 public:
    ~Debugger();
    void init();
    void update(const float renderTime);
 private:
    void performanceWindow();
    std::deque<float> _renderTimes;
};
