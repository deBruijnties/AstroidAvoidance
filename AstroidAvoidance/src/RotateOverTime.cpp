#include "RotateOverTime.h"
#include <Core/Time.h>
#include <Core/Scene/Components/Transform.h>
#include "Core/Math/Types.h"


void RotateOverTime::OnStart()
{

}
Vector3 currentRotation = Vector3(0, 0, 0);
void RotateOverTime::OnUpdate()
{
	currentRotation += rotationSpeed * Time::deltaTime;
	transform->localRotation = Quaternion::FromEuler(Math::Radians(currentRotation));
	transform->MarkDirty();
}


