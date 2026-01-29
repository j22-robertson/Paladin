//
// Created by James Robertson on 29/01/2026.
//

#ifndef PALADIN_RENDERAPPLICATION_H
#define PALADIN_RENDERAPPLICATION_H
#include "IApplication.h"
#include <iostream>


class RenderApplication final : IApplication {
public:
    void run() override;
    void Setup() override;
    bool Update(float delta_time) override;
    void Render(float delta_time) override;
};



#endif //PALADIN_RENDERAPPLICATION_H
