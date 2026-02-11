#include "OrbitCamLook.h"
#include <Core/engine.h>
#include "Core/Math/Types.h"
#include <Core/Scene/Components/Transform.h>
#include <iostream>

float camYaw = 0.0f;
float camPitch = 0.0f;
float camZoom = 250.0f;

void OrbitCamLook::OnStart()
{
    //sets origon obj rotation 
    transform->localPosition = Vector3(0, 0, camZoom);
    // Apply rotation to the parent (orgion)
    Vector3 euler = Math::Radians(Vector3(camPitch, camYaw, 0.0f));


    if (transform->GetParent() == nullptr)
    {
        std::cout << "No parent set\n";
    }

    transform->GetParent()->localRotation = Quaternion::FromEuler(euler);
    transform->GetParent()->MarkDirty();
}

void OrbitCamLook::OnUpdate()
{
    Vector2 d = Input::Mouse::delta;


	// Right-mouse button check for orbiting
    if (Input::Mouse::buttons[1])
    {
        camYaw -= d.x * 0.3f;
        camPitch -= d.y * 0.3f;
        camZoom -= Input::Mouse::scroll.y * 6.0f;

        // Clamp zoom
        if (camZoom > 500) camZoom = 500;
        if (camZoom < 100) camZoom = 100;

        // Clamp pitch
        if (camPitch > 89.0f) camPitch = 89.0f;
        if (camPitch < -89.0f) camPitch = -89.0f;

        //sets origon obj rotation 
        transform->localPosition = Vector3(0, 0, camZoom);
        // Apply rotation to the parent (orgion)
        Vector3 euler = Math::Radians(Vector3(camPitch, camYaw, 0.0f));


        if (transform->GetParent() == nullptr)
        {
            std::cout << "No parent set\n";
        }

        transform->GetParent()->localRotation = Quaternion::FromEuler(euler);
		transform->GetParent()->MarkDirty();
   }
	



}

