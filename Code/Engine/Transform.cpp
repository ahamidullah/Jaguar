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

V3 CalculateTransformForward(Transform *transform)
{
	auto p = DegreesToRadians(transform->rotation.x);
	auto y = DegreesToRadians(transform->rotation.z);
	V3 v =
	{
		Cos(p) * Cos(y),
		Cos(p) * Sin(y),
		Sin(p),
	};
	return Normalize(v);
}

V3 CalculateTransformUp(Transform *transform)
{
	return
	{
		(-Cos(transform->rotation.z) * Sin(transform->rotation.x) * Cos(transform->rotation.y)) + (Sin(transform->rotation.z) * Sin(transform->rotation.y)),
		(-Sin(transform->rotation.z) * Sin(transform->rotation.x) * Cos(transform->rotation.y)) - (Cos(transform->rotation.z) * Sin(transform->rotation.y)),
		Cos(transform->rotation.x) * Sin(transform->rotation.y),
	};
}

V3 CalculateTransformRight(Transform *transform)
{
	return
	{
		(-Cos(transform->rotation.z) * Sin(transform->rotation.x) * Sin(transform->rotation.y)) - (Sin(transform->rotation.z) * Cos(transform->rotation.y)),
		(-Sin(transform->rotation.z) * Sin(transform->rotation.x) * Sin(transform->rotation.y)) + (Cos(transform->rotation.z) * Cos(transform->rotation.y)),
		Cos(transform->rotation.x) * Sin(transform->rotation.y),
	};
}
