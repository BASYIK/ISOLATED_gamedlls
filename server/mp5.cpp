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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "player.h"
#include "soundent.h"
#include "gamerules.h"
#include "paranoia_wpn.h"
enum mp5_e
{
	MP5_IDLE1,
	MP5_IDLE2,
	MP5_DRAW1,
	MP5_DRAW2,
	MP5_FIRE,
	MP5_GRENADE,
	MP5_LASTGRENADE,
	MP5_LOADGRENADE,	// when player picks up his first grenades
	MP5_RELOAD,
};

class CMP5 : public CBaseToggleWeapon
{
	DECLARE_CLASS( CMP5, CBaseToggleWeapon );
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 3; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );
	// Wargon: Фикс невозможности использовать MP5, если у него остались только подствольные гранаты.
	BOOL IsUseable(void);

	void Idle1(void);

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int SecondaryAmmoIndex( void );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );
	float m_flNextAnimTime;
	int m_iShell;
	int m_iGrenadeLoaded;
	float m_flLoadGrenadeTime;
};

LINK_ENTITY_TO_CLASS( weapon_mp5, CMP5 );
LINK_ENTITY_TO_CLASS( weapon_9mmAR, CMP5 );


//=========================================================
//=========================================================
int CMP5::SecondaryAmmoIndex( void )
{
	return m_iSecondaryAmmoType;
}

void CMP5::Spawn( void )
{
	pev->classname = MAKE_STRING("weapon_9mmAR"); // hack to allow for old names
	Precache( );
	SET_MODEL(ENT(pev), "models/w_9mmAR.mdl");
	m_iId = WEAPON_MP5;

	m_iDefaultAmmo = MP5_DEFAULT_GIVE;

	m_iGrenadeLoaded = 1;

	FallInit();// get ready to fall down.
}


void CMP5::Precache( void )
{
	PRECACHE_MODEL("models/v_9mmAR.mdl");
	PRECACHE_MODEL("models/w_9mmAR.mdl");
	PRECACHE_MODEL("models/p_9mmAR.mdl");

	m_iShell = PRECACHE_MODEL("models/mp5_shell.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/grenade.mdl");	// grenade

	PRECACHE_MODEL("models/w_9mmARclip.mdl");
	PRECACHE_SOUND("items/9mmclip1.wav");

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");

	PRECACHE_SOUND("weapons/mp5_fp.wav");// H to the K
	PRECACHE_SOUND("weapons/hks2.wav");// H to the K
	PRECACHE_SOUND("weapons/hks3.wav");// H to the K
	PRECACHE_SOUND("weapons/hks_pinpull.wav");// H to the K

	PRECACHE_SOUND("weapons/glauncher.wav");
	PRECACHE_SOUND("weapons/glauncher2.wav");

	PRECACHE_SOUND("weapons/357_cock1.wav");

	m_flTimeWeaponIdle = 0; // fix to resend idle animation
}

int CMP5::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = _9MM_MAX_CARRY;
	p->pszAmmo2 = "ARgrenades";
	p->iMaxAmmo2 = M203_GRENADE_MAX_CARRY;
	p->iMaxClip = 30;
	p->iSlot = 2;
	p->iPosition = 2;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MP5;
	p->iWeight = MP5_WEIGHT;

	return 1;
}

// Wargon: Фикс невозможности использовать MP5, если у него остались только подствольные гранаты.
BOOL CMP5::IsUseable(void)
{
	if (m_iClip <= 0 && (m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] <= 0 && iMaxAmmo1() != -1) && (m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()] <= 0 && iMaxAmmo2() != -1))
	{
		return FALSE;
	}
	return TRUE;
}


int CMP5::AddToPlayer( CBasePlayer *pPlayer )
{
	if ( CBasePlayerWeapon::AddToPlayer( pPlayer ) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev );
			WRITE_BYTE( m_iId );
		MESSAGE_END();
		return TRUE;
	}
	return FALSE;
}

BOOL CMP5::Deploy( )
{
	InitToggling();

	int iAnim = RANDOM_LONG(0, 1) ? MP5_DRAW1 : MP5_DRAW2;

	return DefaultDeploy( "models/v_9mmAR.mdl", "models/p_9mmAR.mdl", iAnim, "mp5" );
}

void CMP5::PrimaryAttack()
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = gpGlobals->time + 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = gpGlobals->time + 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	SendWeaponAnim( MP5_FIRE );
	m_flNextAnimTime = gpGlobals->time + 0.2;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mp5_fp.wav", 1, ATTN_NORM, 0, 94 + RANDOM_LONG(0,0xf));

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	float spread = ExpandSpread(m_pMySpread->pri_expand);
	EqualizeSpread(&spread, m_pMySpread->pri_equalize);
	Vector vecSpread = AdvanceSpread(m_pMySpread->pri_minspread, m_pMySpread->pri_addspread, spread);
	m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, vecSpread, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed);


	DefPrimPunch();

	m_flNextPrimaryAttack = gpGlobals->time + 0.1;
	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT ( 10, 15 );
	m_flLoadGrenadeTime = gpGlobals->time + 0.5;
}

void CMP5::SecondaryAttack( void )
{
	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = gpGlobals->time + 0.15;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
		PlayEmptySound( );
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
	m_pPlayer->m_flStopExtraSoundTime = gpGlobals->time + 0.2;
			
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
	// buz: play special animation when last grenade fired
	int iAnim = MP5_GRENADE;
	if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
	{
		iAnim = MP5_LASTGRENADE;
		m_iGrenadeLoaded = 0;
		m_flLoadGrenadeTime = UTIL_WeaponTimeBase() + 2.8;
		m_flNextPrimaryAttack = gpGlobals->time + 2.8;
		m_flNextSecondaryAttack = gpGlobals->time + 2.8;
		m_flTimeWeaponIdle = gpGlobals->time + 4;// idle pretty soon after shooting.

	}
	else
	{
		m_flNextPrimaryAttack = gpGlobals->time + 3.4;
		m_flNextSecondaryAttack = gpGlobals->time + 3.4;
		m_flTimeWeaponIdle = gpGlobals->time + 5;// idle pretty soon after shooting.
	}
	SendWeaponAnim(iAnim);

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/glauncher.wav", 0.8, ATTN_NORM);
 
	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	// we don't add in player velocity anymore.
	CGrenade::ShootContact( m_pPlayer->pev, m_pPlayer->EyePosition() + gpGlobals->v_forward * 16, gpGlobals->v_forward * 800 );

	DefSecPunch();
}

void CMP5::Reload( void )
{
	if (DefaultReload(MP5_MAX_CLIP, MP5_RELOAD, 3.5))
	{
		m_flLoadGrenadeTime = gpGlobals->time + 3.5 + 0.1;
	}
}

void CMP5::WeaponIdle( void )
{
	CBaseToggleWeapon::WeaponIdle(); // launch anims, set speed, jump etc

	if (m_flLoadGrenadeTime < UTIL_WeaponTimeBase() && !m_iGrenadeLoaded && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
	{
		SendWeaponAnim(MP5_LOADGRENADE);
		m_flNextPrimaryAttack = gpGlobals->time + 2.6;
		m_flNextSecondaryAttack = gpGlobals->time + 2.6;
		m_flTimeWeaponIdle = gpGlobals->time + 4;// idle pretty soon after shooting.
		m_iGrenadeLoaded = 1;
	}

	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT ( 10, 15 );// how long till we do this again.
}

void CMP5::Idle1(void)
{
	SendWeaponAnim(RANDOM_LONG(0, 1) ? MP5_IDLE1 : MP5_IDLE2);
	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10, 15); // how long till we do this again.
}

class CMP5AmmoClip : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		//		SET_MODEL(ENT(pev), "models/w_9mmARclip.mdl");
		SET_MODEL(ENT(pev), "models/w_mp5ammo.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		//	PRECACHE_MODEL ("models/w_9mmARclip.mdl");
		PRECACHE_MODEL("models/w_mp5ammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_MP5CLIP_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_mp5, CMP5AmmoClip);
LINK_ENTITY_TO_CLASS(ammo_9mmAR, CMP5AmmoClip);



class CMP5Chainammo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		//	SET_MODEL(ENT(pev), "models/w_chainammo.mdl");
		SET_MODEL(ENT(pev), "models/w_mp5ammobox.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		//	PRECACHE_MODEL ("models/w_chainammo.mdl");
		PRECACHE_MODEL("models/w_mp5ammobox.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_CHAINBOX_GIVE, "9mm", _9MM_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_9mmbox, CMP5Chainammo);
LINK_ENTITY_TO_CLASS(ammo_mp5box, CMP5Chainammo);


class CMP5AmmoGrenade : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_ARgrenade.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_ARgrenade.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_M203BOX_GIVE, "ARgrenades", M203_GRENADE_MAX_CARRY) != -1);

		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_mp5grenades, CMP5AmmoGrenade);
LINK_ENTITY_TO_CLASS(ammo_ARgrenades, CMP5AmmoGrenade);



