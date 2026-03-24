#include "RotateOverTime.h"
#include <Core/Time.h>
#include <Core/Scene/Components/Transform.h>
#include "Core/Math/Types.h"


Vector3 currentRotation = Vector3(0, 0, 0);
void RotateOverTime::OnUpdate()
{
	// add to euler angles each frame multiplied by deltatime
	currentRotation += rotationSpeed * Time::deltaTime;
	transform->localRotation = Quaternion::FromEuler(Math::Radians(currentRotation));

	// tell transform to update next frame
	transform->MarkDirty();
}


