#include <iostream>
#include "application/RenderApplication.h"
#include <memory>
int main()
{
    auto app = std::make_unique<RenderApplication>();
    app->run();
    std::cout << "Hello, World!" << std::endl;
    return 0;
}