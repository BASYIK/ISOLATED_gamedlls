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

#include "mp5sd.h"
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

// Animations
enum MP5SDAnimations_e
{
	IDLE = 0,
	DRAW_FIRST,
	DRAW,
	HOLSTER,
	FIRE1,
	FIRE2,
	FIRE3,
	DRYFIRE,
	FIRESELECT,
	RELOAD,
	RELOAD_EMPTY_SLAP,
	RELOAD_EMPTY_PULL,
	RELOAD_EMPTY_PUNCH,
	IRON_IDLE,
	IRON_FIRE1,
	IRON_FIRE2,
	IRON_FIRE3,
	IRON_DRYFIRE,
	IRON_FIRESELECT,
	IRON_TO,
	IRON_FROM
};

CMP5SDWeaponContext::CMP5SDWeaponContext(std::unique_ptr<IWeaponLayer> &&layer) :
	CBaseWeaponContext(std::move(layer))
{
	m_iDefaultAmmo = MP5SD_DEFAULT_GIVE;
	m_iId = WEAPON_MP5SD;
	m_iADSMode = IRON_OUT;
	m_WasDrawn = false;
	m_usFireMP5SD1 = m_pLayer->PrecacheEvent("events/mp5sd.sc");
}

int CMP5SDWeaponContext::GetItemInfo(ItemInfo *p) const
{
	p->pszName = CLASSNAME_STR(MP5SD_CLASSNAME);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = MP5SD_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId;
	p->iWeight = MP5SD_WEIGHT;
	return 1;
}

bool CMP5SDWeaponContext::Deploy()
{
	float deployTime;
	bool bResult;

	if (m_WasDrawn == false)
	{
		bResult = DefaultDeploy("models/weapons/v_mp5sd.mdl", "models/weapons/p_mp5sd.mdl", DRAW_FIRST, "mp5");
		deployTime = 2.33f;
		m_WasDrawn = true;
	}
	else if (m_WasDrawn == true)
	{
		bResult = DefaultDeploy("models/weapons/v_mp5sd.mdl", "models/weapons/p_mp5sd.mdl", DRAW, "mp5");
		deployTime = 1.75f;
	}
	m_pLayer->SetPlayerNextAttackTime(m_pLayer->GetWeaponTimeBase(UsePredicting()) + deployTime);
	m_flTimeWeaponIdle = m_pLayer->GetWeaponTimeBase(UsePredicting()) + deployTime;
	return bResult;
}

void CMP5SDWeaponContext::SecondaryAttack( void )
{
	switch (m_iADSMode)
	{
		case IRON_OUT:
		{
			SendWeaponAnim(IRON_TO);
			AimOn(49);
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.2;
			m_flNextPrimaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.13;
			break;
		}
		case IRON_IN:
		{
			SendWeaponAnim(IRON_FROM);
			AimOff();
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.2;
			m_flNextPrimaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.13;
			break;
		}
	}
}

void CMP5SDWeaponContext::PrimaryAttack( void )
{
	MP5SDFire(0, 0.3, TRUE );
}

void CMP5SDWeaponContext::MP5SDFire( float flSpread , float flCycleTime, bool fUseAutoAim )
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
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

	player->m_iWeaponVolume = QUIET_GUN_VOLUME;
	player->m_iWeaponFlash = DIM_GUN_FLASH;
#endif

	Vector vecSrc = m_pLayer->GetGunPosition();
	matrix3x3 aimMatrix = m_pLayer->GetCameraOrientation();

	if (fUseAutoAim) {
		aimMatrix.SetForward(m_pLayer->GetAutoaimVector(AUTOAIM_10DEGREES));
	}

	Vector vecDir = m_pLayer->FireBullets(1, vecSrc, aimMatrix, 8192, GetSpreadVec(), BULLET_PLAYER_9MM, m_pLayer->GetRandomSeed());
	m_flNextPrimaryAttack = GetNextPrimaryAttackDelay(0.0769f);
	m_flNextSecondaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.1f;

	WeaponEventParams params;
	params.flags = WeaponEventFlags::NotHost;
	params.eventindex = m_usFireMP5SD1;
	params.delay = 0.0f;
	params.origin = vecSrc;
	params.angles = aimMatrix.GetAngles();
	params.fparam1 = vecDir.x;
	params.fparam2 = vecDir.y;
	if (m_iADSMode == IRON_IN)
		params.iparam1 = (m_pLayer->GetRandomInt(m_pLayer->GetRandomSeed(), 0, 1) == 0 ? IRON_FIRE1 : IRON_FIRE3);
	else
		params.iparam1 = (m_pLayer->GetRandomInt(m_pLayer->GetRandomSeed(), 0, 1) == 0 ? FIRE1 : FIRE3);
	params.iparam2 = 0;
	params.bparam1 = (m_iClip == 0) ? 1 : 0;
	params.bparam2 = 0;

	if (m_pLayer->ShouldRunFuncs()) {
		m_pLayer->PlaybackWeaponEvent(params);
	}

	float m_iPunchAngle;


	m_iPunchAngle = (m_iADSMode == IRON_IN) ? m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), -0.75f, -0.60f) : m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), -0.95f, -1.0f);

	m_pLayer->AddPlayerPunchangle(m_iPunchAngle, m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), -0.45, 0.50f), m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), -0.45f, -0.1f));
	m_flTimeWeaponIdle = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 1.0f;
}

void CMP5SDWeaponContext::Reload( void )
{
	int iResult;

	if (m_iADSMode == IRON_IN)
	{
		SendWeaponAnim( IRON_FROM );
		m_pLayer->SetPlayerNextAttackTime(m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.175f);
		AimOff();
	}
	if (m_pLayer->GetPlayerNextAttackTime() < 0.16f)
	{
		if (m_iClip == 0)
			iResult = DefaultReload(17, m_pLayer->GetRandomInt(m_pLayer->GetRandomSeed(), RELOAD_EMPTY_SLAP, RELOAD_EMPTY_PUNCH), 4.28F);
		else
			iResult = DefaultReload(17, RELOAD, 2.85F);
	}

	m_flTimeWeaponIdle = m_pLayer->GetWeaponTimeBase(UsePredicting()) + m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), 10.0f, 15.0f);
}

void CMP5SDWeaponContext::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pLayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if (m_flTimeWeaponIdle > m_pLayer->GetWeaponTimeBase(UsePredicting()))
		return;

	if (m_iADSMode == IRON_IN)
		SendWeaponAnim( IRON_IDLE );
	else
		SendWeaponAnim( IDLE );

	m_flTimeWeaponIdle = m_pLayer->GetWeaponTimeBase(UsePredicting()) + m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), 5.0f, 7.0f);
}
