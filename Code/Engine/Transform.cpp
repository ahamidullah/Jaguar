#include "Transform.h"

V3 Transform::Right()
{
	return this->rotation.Right();
}

V3 Transform::Forward()
{
	return this->rotation.Forward();
}

V3 Transform::Up()
{
	return this->rotation.Up();
}

void Transform::RotateEulerLocal(EulerAngles ea)
{
	RotateTransformsEulerWorld(NewArrayView(&ea, 1), NewArrayView(this, 1));
}

void Transform::RotateEulerWorld(EulerAngles ea)
{
	// @TODO
}

void Transform::RotateAxisAngleLocal(AxisAngle aa)
{
	// @TODO
}

void Transform::RotateAxisAngleWorld(AxisAngle aa)
{
	// @TODO
}

void RotateTransformsEulerLocal(ArrayView<EulerAngles> eas, ArrayView<Transform> out)
{
	Assert(eas.count == out.count);
	for (auto i = 0; i < out.count; i += 1)
	{
		auto e = out[i].rotation.Euler();
		auto yaw = EulerAngles{.yaw = e.yaw + eas[i].yaw}.ToQuaternion();
		auto pitch = EulerAngles{.pitch = e.pitch + eas[i].pitch}.ToQuaternion();
		auto roll = EulerAngles{.roll = e.roll + eas[i].roll}.ToQuaternion();
		out[i].rotation = (yaw * pitch * roll).Normal();
	}
}

void RotateTransformsEulerWorld(ArrayView<EulerAngles> eas, ArrayView<Transform> out)
{
	// @TODO
}

void RotateTransformsAxisAngleLocal(ArrayView<AxisAngle> aas, ArrayView<Transform> out)
{
	// @TODO
}

void RotateTransformsAxisAngleWorld(ArrayView<AxisAngle> aas, ArrayView<Transform> out)
{
	// @TODO
}
