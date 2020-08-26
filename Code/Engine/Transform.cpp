#include "Transform.h"

void TransformRotateEuler(Transform *t, f32 pitch, f32 yaw, f32 roll)
{
	auto angles = QuaternionToEuler(t->rotation);
	auto yawRot = EulerToQuaternion(EulerAngles{.yaw = angles.yaw + yaw});
	auto pitchRot = EulerToQuaternion(EulerAngles{.pitch = angles.pitch + pitch});
	auto rollRot = EulerToQuaternion(EulerAngles{.roll = angles.roll + roll});
	t->rotation = Normalize(yawRot * pitchRot * rollRot);
}
