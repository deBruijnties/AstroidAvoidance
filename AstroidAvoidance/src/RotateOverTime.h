#pragma once
#include <vector>
#include <cstdint>
#include "core/Scene/Component.h"
#include "core/engine.h"


// this compontent can rotate a object's transform overtime used in sun pivot
class RotateOverTime : public Component
{
public:
	Vector3 rotationSpeed = Vector3(0, 0, 0);
    void OnUpdate() override;
};
