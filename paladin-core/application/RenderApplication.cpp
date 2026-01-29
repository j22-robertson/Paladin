//
// Created by James Robertson on 29/01/2026.
//

#include "RenderApplication.h"

void RenderApplication::run() {
    Setup();
}

inline void RenderApplication::Setup() {
    std::cout << "Hello World from Paladin"<< std::endl;
}

bool RenderApplication::Update(float delta_time) {
    return true;
}

void RenderApplication::Render(float delta_time) {

}
