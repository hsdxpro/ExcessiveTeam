#include "TestLevelScript.h"
#include "Core\EngineCore.h"
#include "PlayerScript.h"
#include "ExcessiveStrikeCommon.h"
#include "mymath\mymath.h"


TestLevelScript::TestLevelScript()
{
	//Actor* plane = World.AddActor("box.DAE", 0);
	//plane->SetKinematic(true);
	//plane->SetScale(mm::vec3(100, 100, 1));
	//plane->SetCollisionGroup(eES_CollisionGroup::GROUND);

	//Add ground to
	Actor* ground = World.AddActor("Terminal/terminal_blender.dae", 0);
	ground->RotX(90);
	ground->SetCollisionGroup(eES_CollisionGroup::GROUND);
	ground->SetName("ground");

	//Add sky to game
	//Actor* sky = World.AddActor_Mesh("skybox.dae");
	//sky->SetScale(1000);
	//sky->RotX(90);

	//// Set up collision layers..
	Physics.SetLayeCollision(eES_CollisionGroup::PLAYER, eES_CollisionGroup::SHELL, false);
	Physics.SetLayeCollision(eES_CollisionGroup::GROUND, eES_CollisionGroup::GROUND, false);
	Physics.SetLayeCollision(eES_CollisionGroup::SHELL, eES_CollisionGroup::SHELL, false);
	Physics.SetLayeCollision(eES_CollisionGroup::SHELL, eES_CollisionGroup::PLAYER, false);
	Physics.SetLayeCollision(eES_CollisionGroup::BULLET, eES_CollisionGroup::SHELL, false);
}

void TestLevelScript::Update(float deltaSeconds)
{
	playerScript.Update(deltaSeconds);
}