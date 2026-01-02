/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "pkm.h"
#include <utility>

#ifdef CLIENT_DLL
#else
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#endif
#ifdef MACHINEGUNS_ALLOWED
CPKMWeaponContext::CPKMWeaponContext(std::unique_ptr<IWeaponLayer> &&layer) :
	CBaseWeaponContext(std::move(layer))
{
	m_iDefaultAmmo = PKM_DEFAULT_GIVE;
	m_iId = WEAPON_PKM;
	m_iADSMode = IRON_OUT;
	m_WasDrawn = false;
	m_usFirePKM1 = m_pLayer->PrecacheEvent("events/pkm.sc");
}

int CPKMWeaponContext::GetItemInfo(ItemInfo *p) const
{
	p->pszName = CLASSNAME_STR(PKM_CLASSNAME);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = PKM_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId;
	p->iWeight = PKM_WEIGHT;
	return 1;
}

bool CPKMWeaponContext::Deploy()
{
	float deployTime;
	bool bResult;

	if (m_WasDrawn == false)
	{
		bResult = DefaultDeploy("models/weapons/v_pkm.mdl", "models/weapons/p_pkm.mdl", PKM_DRAW_FIRST, "mp5");
		deployTime = 2.33f;
		m_WasDrawn = true;
	}
	else if (m_WasDrawn == true)
	{
		bResult = DefaultDeploy("models/weapons/v_pkm.mdl", "models/weapons/p_pkm.mdl", PKM_DRAW, "mp5");
		deployTime = 1.75f;
	}
	m_pLayer->SetPlayerNextAttackTime(m_pLayer->GetWeaponTimeBase(UsePredicting()) + deployTime);
	m_flTimeWeaponIdle = m_pLayer->GetWeaponTimeBase(UsePredicting()) + deployTime;
	return bResult;
}

void CPKMWeaponContext::SecondaryAttack( void )
{
	switch (m_iADSMode)
	{
		case IRON_OUT:
		{
			SendWeaponAnim(PKM_IRON_TO);
			AimOn(49);
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.2;
			m_flNextPrimaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.13;
			break;
		}
		case IRON_IN:
		{
			SendWeaponAnim(PKM_IRON_FROM);
			AimOff();
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.2;
			m_flNextPrimaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.13;
			break;
		}
	}
}

void CPKMWeaponContext::PrimaryAttack( void )
{
	PKMFire(0, 0.3, TRUE );
}

void CPKMWeaponContext::PKMFire( float flSpread , float flCycleTime, bool fUseAutoAim )
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			SendWeaponAnim(m_iADSMode == IRON_IN ? PKM_IRON_SHOOT_EMPTY : PKM_SHOOT_EMPTY);
			m_flNextPrimaryAttack = GetNextPrimaryAttackDelay(0.33f);
		}

		return;
	}

	m_iClip--;


#ifndef CLIENT_DLL
	// player "shoot" animation
	CBasePlayer *player = m_pLayer->GetWeaponEntity()->m_pPlayer;

	player->SetAnimation( PLAYER_ATTACK1 );
	player->pev->effects = (int)(player->pev->effects) | EF_MUZZLEFLASH;

	// silenced
	if (m_pLayer->GetWeaponBodygroup() == 1)
	{
		player->m_iWeaponVolume = QUIET_GUN_VOLUME;
		player->m_iWeaponFlash = DIM_GUN_FLASH;
	}
	else
	{
		// non-silenced
		player->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		player->m_iWeaponFlash = NORMAL_GUN_FLASH;
	}
#endif

	Vector vecSrc = m_pLayer->GetGunPosition();
	matrix3x3 aimMatrix = m_pLayer->GetCameraOrientation();

	if (fUseAutoAim) {
		aimMatrix.SetForward(m_pLayer->GetAutoaimVector(AUTOAIM_10DEGREES));
	}

	Vector vecDir = m_pLayer->FireBullets(1, vecSrc, aimMatrix, 8192, GetSpreadVec(), BULLET_PLAYER_9MM, m_pLayer->GetRandomSeed());
	m_flNextPrimaryAttack = GetNextPrimaryAttackDelay(0.1f);
	m_flNextSecondaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.1f;

	WeaponEventParams params;
	params.flags = WeaponEventFlags::NotHost;
	params.eventindex = m_usFirePKM1;
	params.delay = 0.0f;
	params.origin = vecSrc;
	params.angles = aimMatrix.GetAngles();
	params.fparam1 = vecDir.x;
	params.fparam2 = vecDir.y;
	if (m_iADSMode == IRON_IN)
		params.iparam1 = (m_pLayer->GetRandomInt(m_pLayer->GetRandomSeed(), 0, 1) == 0 ? PKM_IRON_SHOOT1 : PKM_IRON_SHOOT3);
	else
		params.iparam1 = (m_pLayer->GetRandomInt(m_pLayer->GetRandomSeed(), 0, 1) == 0 ? PKM_SHOOT1 : PKM_SHOOT3);
	params.iparam2 = 0;
	params.bparam1 = (m_iClip == 0) ? 1 : 0;
	params.bparam2 = 0;

	if (m_pLayer->ShouldRunFuncs()) {
		m_pLayer->PlaybackWeaponEvent(params);
	}

	float m_iPunchAngle;


	m_iPunchAngle = (m_iADSMode == IRON_IN) ? m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), -1.75f, -1.50f) : m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), -2.75f, -2.50f);

	m_pLayer->AddPlayerPunchangle(m_iPunchAngle, m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), -0.75, 0.50f), m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), -0.5f, -0.1f));
	m_flTimeWeaponIdle = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 1.0f;
}

void CPKMWeaponContext::Reload( void )
{
	int iResult;

	if (m_iADSMode == IRON_IN)
	{
		SendWeaponAnim( PKM_IRON_FROM );
		m_pLayer->SetPlayerNextAttackTime(m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.175f);
		AimOff();
	}
	if (m_pLayer->GetPlayerNextAttackTime() < 0.16f)
	{
		if (m_iClip == 0)
			iResult = DefaultReload(17, PKM_RELOAD_EMPTY_1, 4.77F);
		else
			iResult = DefaultReload(17, PKM_RELOAD, 2.275F);
	}
	if (iResult)
	{
		m_flTimeWeaponIdle = m_pLayer->GetWeaponTimeBase(UsePredicting()) + m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), 10.0f, 15.0f);
	}
}

void CPKMWeaponContext::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pLayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if (m_flTimeWeaponIdle > m_pLayer->GetWeaponTimeBase(UsePredicting()))
		return;

	if (m_iADSMode == IRON_IN)
		SendWeaponAnim( PKM_IRON_IDLE );
	else
		SendWeaponAnim( PKM_IDLE );

	m_flTimeWeaponIdle = m_pLayer->GetWeaponTimeBase(UsePredicting()) + m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), 5.0f, 7.0f);
}
#endif