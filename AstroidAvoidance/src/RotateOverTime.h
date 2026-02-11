#pragma once

#include <vector>
#include <cstdint>
#include "core/engine.h"

#include <core/Scene/Component.h>

class RotateOverTime : public Component
{
public:
	Vector3 rotationSpeed = Vector3(0, 0, 0);
    void OnStart() override;
    void OnUpdate() override;
};
