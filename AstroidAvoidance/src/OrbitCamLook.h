#pragma once
#include <Core/Scene/Component.h>


// rotate around orgin and change distance from it
class OrbitCamLook : public Component
{
public:
    void OnStart() override;
    void OnUpdate() override;
};


