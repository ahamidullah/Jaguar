#pragma once

#include "Math.h"
#include "Basic/Array.h"

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

void RotateTransformsEulerLocal(ArrayView<EulerAngles> eas, ArrayView<Transform> out);
void RotateTransformsEulerWorld(ArrayView<EulerAngles> eas, ArrayView<Transform> out);
void RotateTransformsAxisAngleLocal(ArrayView<AxisAngle> aas, ArrayView<Transform> out);
void RotateTransformsAxisAngleWorld(ArrayView<AxisAngle> aas, ArrayView<Transform> out);
