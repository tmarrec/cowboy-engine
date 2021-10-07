#pragma once

#include "Window.h"

#include <chrono>
#include <memory>

class Core
{
 public:
     int Run();

 private:
     void RegisterAllComponents() const;

     std::unique_ptr<Window> _window = std::make_unique<Window>();
};
