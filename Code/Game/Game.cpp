#include "Engine/Engine.h"

void InitalizeGame()
{
	auto e = CreateEntity();
	auto t = Transform{};
	SetEntityTransform(e, t);
	SetEntityModel(e, ANVIL_ASSET, t);
}

void GameLoop(f32 deltaTime)
{
	ConsolePrint("WHAT\n");
}
