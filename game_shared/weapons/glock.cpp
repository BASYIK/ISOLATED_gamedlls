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

#include "glock.h"
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

CGlockWeaponContext::CGlockWeaponContext(std::unique_ptr<IWeaponLayer> &&layer) :
	CBaseWeaponContext(std::move(layer))
{
	m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;
	m_iId = WEAPON_GLOCK;
	m_usFireGlock1 = m_pLayer->PrecacheEvent("events/glock1.sc");
	m_usFireGlock2 = m_pLayer->PrecacheEvent("events/glock2.sc");
}

int CGlockWeaponContext::GetItemInfo(ItemInfo *p) const
{
	p->pszName = CLASSNAME_STR(GLOCK_CLASSNAME);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GLOCK_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId;
	p->iWeight = GLOCK_WEIGHT;
	return 1;
}

bool CGlockWeaponContext::Deploy( )
{
	return DefaultDeploy( "models/ins2/wpn/g17/v_g17.mdl", "models/ins2/wpn/g17/p_g17.mdl", DRAW, "onehanded" );
}

void CGlockWeaponContext::SecondaryAttack( void )
{
	m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.2;
	m_flNextPrimaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.13;

	switch (m_iADSMode)
	{
		case IRON_OUT:
		{
			SendWeaponAnim((m_iClip > 0) ? IRON_TO : IRON_TO_EMPTY);
			AimOn(49);
			break;
		}
		case IRON_IN:
		{
			SendWeaponAnim((m_iClip > 0) ? IRON_FROM : IRON_FROM_EMPTY);
			AimOff();
			break;
		}
	}
}

void CGlockWeaponContext::PrimaryAttack( void )
{
	GlockFire( 0.035, 0.3, TRUE );
}

void CGlockWeaponContext::GlockFire( float flSpread , float flCycleTime, bool fUseAutoAim )
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			SendWeaponAnim(m_iADSMode == IRON_IN ? IRON_DRYFIRE : DRYFIRE);
			m_flNextPrimaryAttack = GetNextPrimaryAttackDelay(0.2f);
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

	Vector vecDir = m_pLayer->FireBullets(1, vecSrc, aimMatrix, 8192, flSpread, BULLET_PLAYER_9MM, m_pLayer->GetRandomSeed());
	m_flNextPrimaryAttack = GetNextPrimaryAttackDelay(0.125f);
	m_flNextSecondaryAttack = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.125f;

	if (m_iADSMode == IRON_IN)
		SendWeaponAnim((m_iClip > 0) ? (m_pLayer->GetRandomInt(m_pLayer->GetRandomSeed(), 0, 1) == 0 ? IRON_FIRE1 : IRON_FIRE4) : IRON_FIRE_LAST);
	else
		SendWeaponAnim((m_iClip > 0) ? (m_pLayer->GetRandomInt(m_pLayer->GetRandomSeed(), 0, 1) == 0 ? FIRE1 : FIRE3) : FIRE_LAST);

	WeaponEventParams params;
	params.flags = WeaponEventFlags::NotHost;
	params.eventindex = m_iADSMode == IRON_IN ? m_usFireGlock2 : m_usFireGlock1;
	params.delay = 0.0f;
	params.origin = vecSrc;
	params.angles = aimMatrix.GetAngles();
	params.fparam1 = vecDir.x;
	params.fparam2 = vecDir.y;
	params.iparam1 = 0;
	params.iparam2 = 0;
	params.bparam1 = (m_iClip == 0) ? 1 : 0;
	params.bparam2 = 0;

	if (m_pLayer->ShouldRunFuncs()) {
		m_pLayer->PlaybackWeaponEvent(params);
	}

	m_pLayer->AddPlayerPunchangle(m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), -2.55, -3.6), (m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), 0, 1) < 0.5) ? -0.95f : 1.25f, m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), -0.5, 0.5));
	m_flTimeWeaponIdle = m_pLayer->GetWeaponTimeBase(UsePredicting()) + 1.0f;
}

void CGlockWeaponContext::Reload( void )
{
	int iResult;

	if (m_iADSMode == IRON_IN)
	{
		SendWeaponAnim((m_iClip > 0) ? IRON_FROM : IRON_FROM_EMPTY);
		m_pLayer->SetPlayerNextAttackTime(m_pLayer->GetWeaponTimeBase(UsePredicting()) + 0.16f);
		AimOff();
	}
	if (m_pLayer->GetPlayerNextAttackTime() < 0.16f)
	{
		if (m_iClip == 0)
			iResult = DefaultReload(17, RELOAD_EMPTY, 2.75);
		else
			iResult = DefaultReload(17, RELOAD, 2.2);
	}
	if (iResult)
	{
		m_flTimeWeaponIdle = m_pLayer->GetWeaponTimeBase(UsePredicting()) + m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), 10.0f, 15.0f);
	}
}

void CGlockWeaponContext::WeaponIdle( void )
{
	ResetEmptySound( );

	m_pLayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if (m_flTimeWeaponIdle > m_pLayer->GetWeaponTimeBase(UsePredicting()))
		return;

	if (m_iADSMode == IRON_IN)
		SendWeaponAnim((m_iClip > 0) ? IRON_IDLE : IRON_IDLE_EMPTY);
	else
		SendWeaponAnim((m_iClip > 0) ? IDLE : IDLE_EMPTY);

	m_flTimeWeaponIdle = m_pLayer->GetWeaponTimeBase(UsePredicting()) + m_pLayer->GetRandomFloat(m_pLayer->GetRandomSeed(), 5.0f, 7.0f);
}
