﻿#include "EngineCore.h"
#include "Factory.h"

// basic util
#include <chrono>
#include <thread>
#include <iostream>
#include <string>
#include <sstream>
#include "Sys.h"


#define CAM_MOVE_SPEED 5
float gCamSpeedMultiplier = 1;

IGapi* gGapi;

int Ricsi() 
{
	// Init window
	rWindow d;
		d.clientW = 800;
		d.clientH = 600;
		d.capText = "Excessive-Engine -> Ricsi teszt";
	IWindow* window = Factory::createWindow(d);

	// Init engine core (graphics, physics, sound, network
	EngineCore core;
		core.initSoundEngine();
		core.initNetworkEngine();
		core.initPhysicsEngineBullet();
			rGraphicsEngineRaster gDesc;
			gDesc.gapiType = eGapiType::OPENGL_4_5;
			gDesc.targetWindow = window;
		graphics::IEngine* gEngine = core.initGraphicsEngineRaster(gDesc);

	// Create camera
	graphics::ICamera* cam = core.addCompCamera()->getCam();
	cam->setFOV(70 / 180.f*3.1415926f);
	//cam->setAspectRatio(window->getClientAspectRatio());
	cam->setNearPlane(0.2f);
	cam->setFarPlane(2000);
	cam->setPos(mm::vec3(0, -3, 1));

	// Create scene, plug camera
	graphics::IScene* scene = core.getDefaultGraphicsScene();
	scene->setCamera(cam);

	static const wchar_t assetName[] = L"../Assets/demo_ground.dae"; // Assets/terminal/terminal.dae
	static const wchar_t teapotModelPath[] = L"../Assets/box.dae"; // Assets/teapot.dae
	static const wchar_t ak47ModelPath[] = L"../Assets/ak47/ak.obj"; // Assets/teapot.dae

	core.addCompRigidBodyFromFile(Sys::getWorkDir() + assetName, 0)->addChild(core.addCompGraphicsFromFile(Sys::getWorkDir() + assetName));
	core.addCompGraphicsFromFile(Sys::getWorkDir() + L"../Assets/skybox.dae")->setScale({ 1000, 1000, 1000 });

	// Run the main loop
	rWindowEvent ev;
	double elapsed;
	double last_frame;

	// Timer for dT frame calc
	ITimer* timer = Factory::createTimer();
	timer->start();
	last_frame = 0;
	elapsed = 0;

	bool bWDown = false;
	bool bSDown = false;
	bool bADown = false;
	bool bDDown = false;

	bool bRMBDown = false;

	// Jesus, just because we dont want garbage for last_frame variable at first run
	static bool bFirstRun = true;

	while (window->isOpen())
	{
		// keep 60 fps
		double now = timer->getSecondsPassed();

		if (bFirstRun) 
		{
			bFirstRun = false;
			last_frame = now;
		}
		elapsed = now - last_frame;
		last_frame = now;

		int fps = 1 / elapsed + 0.5;
		std::wstringstream title_ss;
		static float camAngleX = 0; // upwards/downwards looking
		static float camAngleZ = 0; // orientation
		title_ss << L"Excessive-Engine - Ricsi teszt | FPS=" << fps << L" | Pitch=" << camAngleX * 180 / 3.141592653f << L"° Facing=" << camAngleZ * 180 / 3.141592653f << L"°";
		window->setText(title_ss.str().c_str());

		while (window->popEvent(&ev))
		switch (ev.msg)
		{
			case eWindowMsg::MOUSE_PRESS:
				if (ev.mouseBtn == eMouseBtn::RIGHT)
					bRMBDown = true;
				else if (ev.mouseBtn == eMouseBtn::LEFT)
				{
					auto box = core.addCompRigidBodyFromFile(Sys::getWorkDir() + teapotModelPath, 10);
					box->addChild(core.addCompGraphicsFromFile(Sys::getWorkDir() + teapotModelPath));

					box->setPos(cam->getPos() + cam->getDirNormedFront() * 3); // 3 méterrel elénk
					box->setScale(mm::vec3(1.f / 20, 1.f / 20, 1.f / 20));
				}
				break;
			case eWindowMsg::MOUSE_RELEASE:
				if (ev.mouseBtn == eMouseBtn::RIGHT)
					bRMBDown = false;
				break;
			case eWindowMsg::MOUSE_MOVE: 
			{
				if (bRMBDown)
				{
					float angleChangeZ = (float)(ev.deltaX) * 0.009;
					float angleChangeX = (float)(-ev.deltaY) * 0.009;

					mm::vec3 viewDir = mm::normalize(cam->getTargetPos() - cam->getPos());
					float lenXY = mm::length(viewDir.xy);
					static float angleX = acos(lenXY)*(viewDir.z > 0 ? 1 : -1);
					angleX += angleChangeX;
					angleX = std::max(-85.f / 180 * 3.141592653f, std::min(angleX, 85.f / 180 * 3.141592653f));
					static float angleZ = atan2(viewDir.y / lenXY, viewDir.z / lenXY) - 3.141592653f / 2;
					angleZ += angleChangeZ;
					if (angleZ > 3.141592653f)
						angleZ -= floor(angleZ / 3.141592653f) * 2 * 3.141592653f;
					else if (angleZ < -3.141592653f)
						angleZ -= ceil(angleZ / 3.141592653f) * 2 * 3.141592653f;

					mm::vec3 newViewDir(0, 1, 0);

					// MY HEKK
					angleZ = 3.1415 / 2;

					mm::mat3 rotAroundX(1,		0,			0,
										0, cos(angleX), -sin(angleX),
										0, sin(angleX),  cos(angleX));

					mm::mat3 rotAroundZ(cos(-angleZ), -sin(-angleZ),	0,
										sin(-angleZ),  cos(-angleZ),	0,
											0,			0,				1);

					//newViewDir *= rotAroundX;
					newViewDir *= rotAroundZ;

					camAngleX = angleX;
					camAngleZ = angleZ;

					cam->setTarget(cam->getPos() + newViewDir);
				}
			} break;

			case eWindowMsg::KEY_PRESS:
				switch (ev.key)
				{
					case eKey::ESCAPE: window->close(); break;
					case eKey::W: bWDown = true; break;
					case eKey::S: bSDown = true; break;
					case eKey::A: bADown = true; break;
					case eKey::D: bDDown = true; break;
					case eKey::LSHIFT: gCamSpeedMultiplier = 5; break;
				} break;
			case eWindowMsg::KEY_RELEASE:
				switch (ev.key)
				{
					case eKey::W: bWDown = false; break;
					case eKey::S: bSDown = false; break;
					case eKey::A: bADown = false; break;
					case eKey::D: bDDown = false; break;
					case eKey::LSHIFT: gCamSpeedMultiplier = 1; break;
				} break;

			case eWindowMsg::RESIZE:
				//cam->setAspectRatio((float)ev.x / (float)ev.y);
				break;
		}

		// Camera move
		if (bWDown) // W
			cam->setPos(cam->getPos() + cam->getDirNormedFront() * CAM_MOVE_SPEED * elapsed * gCamSpeedMultiplier);
		if (bSDown) // S									 
			cam->setPos(cam->getPos() + cam->getDirNormedBack()  * CAM_MOVE_SPEED * elapsed * gCamSpeedMultiplier);
		if (bADown) // A									 
			cam->setPos(cam->getPos() + cam->getDirNormedLeft()  * CAM_MOVE_SPEED * elapsed * gCamSpeedMultiplier);
		if (bDDown) // D									 
			cam->setPos(cam->getPos() + cam->getDirNormedRight() * CAM_MOVE_SPEED * elapsed * gCamSpeedMultiplier);

		// Update core
		core.update(elapsed/*, scene*/);

		window->present();
	}
	std::cout << std::endl;

	return 0;
}