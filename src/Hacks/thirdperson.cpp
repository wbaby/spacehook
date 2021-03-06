#include "thirdperson.h"

bool Settings::ThirdPerson::enabled = false;
float Settings::ThirdPerson::distance = 30.f;

void ThirdPerson::OverrideView(CViewSetup *pSetup)
{
	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());

	if(!localplayer)
		return;

	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());

	if (activeWeapon && activeWeapon->GetCSWpnData() && activeWeapon->GetCSWpnData()->GetWeaponType() == CSWeaponType::WEAPONTYPE_GRENADE)
	{
		input->m_fCameraInThirdPerson = false;
		return;
	}

	if(localplayer->GetAlive() && Settings::ThirdPerson::enabled && !engine->IsTakingScreenshot())
	{
		QAngle viewAngles;
		engine->GetViewAngles(viewAngles);
		trace_t tr;
		Ray_t traceRay;
		Vector eyePos = localplayer->GetEyePosition();

		Vector camOff = Vector(cos(DEG2RAD(viewAngles.y)) * Settings::ThirdPerson::distance,
							   sin(DEG2RAD(viewAngles.y)) * Settings::ThirdPerson::distance,
							   sin(DEG2RAD(-viewAngles.x)) * Settings::ThirdPerson::distance);

		traceRay.Init(eyePos, (eyePos - camOff));
		CTraceFilter traceFilter;
		traceFilter.pSkip = localplayer;
		trace->TraceRay(traceRay, MASK_SHOT, &traceFilter, &tr);

		Vector eyeDifference = eyePos - tr.endpos;

		float distance2D = sqrt(abs(eyeDifference.x * eyeDifference.x) + abs(eyeDifference.y * eyeDifference.y));

		bool dis2Dbool = distance2D > (Settings::ThirdPerson::distance - 2.0f);
		bool eyebool = (abs(eyeDifference.z) - abs(camOff.z) < 3.0f);

		float cameraDistance;

		if(dis2Dbool && eyebool)
			cameraDistance = Settings::ThirdPerson::distance;

		else if(eyebool) cameraDistance = distance2D * 0.95f;
		else cameraDistance = abs(eyeDifference.z) * 0.95f;

		if(!input->m_fCameraInThirdPerson)
			input->m_fCameraInThirdPerson = true;

		input->m_vecCameraOffset = Vector(viewAngles.x, viewAngles.y, cameraDistance);

		localplayer->UpdateVisibility();

		C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
		if (activeWeapon)
			activeWeapon->UpdateVisibility();

		/*for (size_t i = 0; localplayer->GetWeapons()[i] != (int) 0xFFFFFFFF; i++) // doesn't work
        {
            if (localplayer->GetWeapons()[i] == -1)
                continue;

            C_BaseAttributableItem *weapon = (C_BaseAttributableItem *) entityList->GetClientEntity(localplayer->GetWeapons()[i] & 0xFFF);
            if (!weapon)
                continue;

            weapon->UpdateVisibility();
        }*/
	}
	else if(input->m_fCameraInThirdPerson)
	{
		input->m_fCameraInThirdPerson = false;
		input->m_vecCameraOffset = Vector(0.f, 0.f, 0.f);
	}
}

void ThirdPerson::FrameStageNotify(ClientFrameStage_t stage)
{
	if (stage == ClientFrameStage_t::FRAME_RENDER_START && engine->IsInGame())
	{
		C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());

		if (localplayer && localplayer->GetAlive() && Settings::ThirdPerson::enabled && input->m_fCameraInThirdPerson)
		{
			*localplayer->GetVAngles() = AntiAim::real;
		}
	}
}