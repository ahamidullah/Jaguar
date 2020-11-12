#pragma once

#include "Math.h"
#include "Basic/Container/Array.h"

struct Transform
{
	V3 position;
	Quaternion rotation;

	V3 Right();
	V3 Forward();
	V3 Up();
	void RotateEulerLocal(EulerAngles ea);
	void RotateEulerWorld(EulerAngles ea);
	void RotateAxisAngleLocal(AxisAngle aa);
	void RotateAxisAngleWorld(AxisAngle aa);
};

void RotateTransformsEulerLocal(array::View<EulerAngles> eas, array::View<Transform> out);
void RotateTransformsEulerWorld(array::View<EulerAngles> eas, array::View<Transform> out);
void RotateTransformsAxisAngleLocal(array::View<AxisAngle> aas, array::View<Transform> out);
void RotateTransformsAxisAngleWorld(array::View<AxisAngle> aas, array::View<Transform> out);
