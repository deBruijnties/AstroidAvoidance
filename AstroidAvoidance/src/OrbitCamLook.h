#pragma once
#include <Core/Scene/Component.h>



class OrbitCamLook : public Component
{
public:
    void OnStart() override;
    void OnUpdate() override;
};


