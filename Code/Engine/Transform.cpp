void SetTransformRotation(Transform *transform, f32 pitch, f32 roll, f32 yaw)
{
	transform->rotation.x = pitch;
	transform->rotation.y = roll;
	transform->rotation.z = yaw;
}

void SetTransformPosition(Transform *transform, f32 x, f32 y, f32 z)
{
	transform->position.x = x;
	transform->position.y = y;
	transform->position.z = z;
}
