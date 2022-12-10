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
#include "gamerules.h"

#define CROWBAR_BODYHIT_VOLUME	128
#define CROWBAR_WALLHIT_VOLUME	512
#define PARANOIA
// Wargon: »зменено после правки анимаций и эвентов в модели. (1.2)
enum crowbar_e {
	CROWBAR_IDLE = 0,
	CROWBAR_ATTACK1HIT,
	CROWBAR_ATTACK2HIT,
	CROWBAR_SEC,
	CROWBAR_SECMISS,
	CROWBAR_DRAW
};


class CCrowbar : public CBasePlayerWeapon
{
	DECLARE_CLASS( CCrowbar, CBasePlayerWeapon );
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 1; }
	void SwingAgain( void );
	void Smack( void );
	int GetItemInfo(ItemInfo *p);

	DECLARE_DATADESC();
#if 1
	int Swing(int iSecondary);
	void SecondaryAttack(void);
#else
	int Swing(int fFirst);
#endif
	void PrimaryAttack( void );
	BOOL Deploy( void );
	void Holster( void );
	int m_iSwing;
	TraceResult m_trHit;
};

LINK_ENTITY_TO_CLASS( weapon_crowbar, CCrowbar );

BEGIN_DATADESC( CCrowbar )
	DEFINE_FUNCTION( SwingAgain ),
	DEFINE_FUNCTION( Smack ),
END_DATADESC()

void CCrowbar::Spawn( void )
{
	pev->classname = MAKE_STRING("weapon_crowbar");

	Precache( );
	m_iId = WEAPON_CROWBAR;
	SET_MODEL(ENT(pev), "models/w_crowbar.mdl");
	m_iClip = -1;

	FallInit();// get ready to fall down.
}

void CCrowbar::Precache( void )
{
	PRECACHE_MODEL("models/v_crowbar.mdl");
	PRECACHE_MODEL("models/w_crowbar.mdl");
	PRECACHE_MODEL("models/p_crowbar.mdl");
	PRECACHE_SOUND("weapons/cbar_hit1.wav");
	PRECACHE_SOUND("weapons/cbar_hit2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	// Wargon: «вук попадани¤ вторичной атакой. (1.2)
	PRECACHE_SOUND("weapons/knife_stab.wav");
}

int CCrowbar::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iId = WEAPON_CROWBAR;
	p->iWeight = CROWBAR_WEIGHT;
	return 1;
}

BOOL CCrowbar::Deploy( )
{
	return DefaultDeploy( "models/v_crowbar.mdl", "models/p_crowbar.mdl", CROWBAR_DRAW, "knife" );
}

void CCrowbar::Holster( void )
{
	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
	SendWeaponAnim(CROWBAR_IDLE);
}

void FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
{
	int			i, j, k;
	float		distance;
	float		*minmaxs[2] = {mins, maxs};
	TraceResult tmpTrace;
	Vector		vecHullEnd = tr.vecEndPos;
	Vector		vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc)*2);
	UTIL_TraceLine( vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace );
	if ( tmpTrace.flFraction < 1.0 )
	{
		tr = tmpTrace;
		return;
	}

	for ( i = 0; i < 2; i++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			for ( k = 0; k < 2; k++ )
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace );
				if ( tmpTrace.flFraction < 1.0 )
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if ( thisDistance < distance )
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}


void CCrowbar::PrimaryAttack( void )
{
#ifdef PARANOIA
	Swing(0);
#else
	if (! Swing( 1 ))
	{
		SetThink( &CCrowbar::SwingAgain );
		pev->nextthink = gpGlobals->time + 0.1;
	}
#endif
}
#ifdef PARANOIA
void CCrowbar::SecondaryAttack()
{
	Swing(1);
}
#endif
void CCrowbar::Smack( void )
{
	UTIL_StudioDecalTrace(&m_trHit, "knife1");
	UTIL_TraceCustomDecal(&m_trHit, "knife1");
}

void CCrowbar::SwingAgain( void )
{
	Swing( 0 );
}
#ifdef PARANOIA

int CCrowbar::Swing(int iSecondary)
{
	int fDidHit = FALSE;

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}

	if (tr.flFraction >= 1.0)
	{
		EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_miss1.wav", 1, ATTN_NORM); // buz

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		// buz: наверное, код фомки писалс¤ вальвовцами в состо¤нии бреда.. просто скопирую, и не буду парить мозг
		if (iSecondary)
		{
			SendWeaponAnim(CROWBAR_SECMISS);
			m_pPlayer->ViewPunch(-4, 0, 0);
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + 0.6;
		}
		else
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + 0.5;
			if (m_iSwing == 0)
			{
				SendWeaponAnim(CROWBAR_ATTACK1HIT);
				m_pPlayer->ViewPunch(0.5, -0.5, 0);
				m_iSwing = 1;
			}
			else
			{
				SendWeaponAnim(CROWBAR_ATTACK2HIT);
				m_pPlayer->ViewPunch(0.5, 0.5, 0);
				m_iSwing = 0;
			}
		}
		//	}
	}
	else
	{
		/*	switch( ((m_iSwing++) % 2) + 1 )
			{
			case 0:
				SendWeaponAnim( CROWBAR_ATTACK1HIT ); break;
			case 1:
				SendWeaponAnim( CROWBAR_ATTACK2HIT ); break;
			case 2:
				SendWeaponAnim( CROWBAR_ATTACK3HIT ); break;
			}*/

		if (iSecondary)
		{
			SendWeaponAnim(CROWBAR_SEC);
			m_pPlayer->ViewPunch(-4, 0, 0);
		}
		else
		{
			if (m_iSwing == 0)
			{
				SendWeaponAnim(CROWBAR_ATTACK1HIT);
				m_pPlayer->ViewPunch(2.5, -2.5, 0);
				m_iSwing = 1;
			}
			else
			{
				SendWeaponAnim(CROWBAR_ATTACK2HIT);
				m_pPlayer->ViewPunch(0.5, 2.5, 0);
				m_iSwing = 0;
			}
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		// hit
		fDidHit = TRUE;
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage();

		/*	if ( (m_flNextPrimaryAttack + 1 < UTIL_WeaponTimeBase() ) || g_pGameRules->IsMultiplayer() )
			{
				// first swing does full damage
				pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgCrowbar, gpGlobals->v_forward, &tr, DMG_CLUB );
			}
			else
			{
				// subsequent swings do half
				pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgCrowbar / 2, gpGlobals->v_forward, &tr, DMG_CLUB );
			}*/

			// Wargon: »справлено гибание ножем.
		if (iSecondary)
			pEntity->TraceAttack(m_pPlayer->pev, 40, gpGlobals->v_forward, &tr, DMG_CLUB | DMG_NEVERGIB); // buz
		else
			pEntity->TraceAttack(m_pPlayer->pev, 15, gpGlobals->v_forward, &tr, DMG_CLUB | DMG_NEVERGIB); // buz

		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		int fHitWorld = TRUE;

		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				// Wargon: «вук попадани¤ вторичной атакой теперь воспроизводитс¤ из кода, а не из эвента в модели. (1.2)
				if (iSecondary)
				{
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knife_stab.wav", 1, ATTN_NORM);
				}
				else
				{
					switch (RANDOM_LONG(0, 2))
					{
					case 0:
						EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod1.wav", 1, ATTN_NORM); break;
					case 1:
						EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod2.wav", 1, ATTN_NORM); break;
					case 2:
						EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod3.wav", 1, ATTN_NORM); break;
					}
				}

				m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
				{
					m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + 0.37; //LRC: corrected half-life bug
					return TRUE;
				}
				else
					flVol = 0.1;

				fHitWorld = FALSE;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line
		if (fHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

			if (g_pGameRules->IsMultiplayer())
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play crowbar strike
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			case 1:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;

		if (iSecondary)
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + 0.65;
			SetThink(&CCrowbar::Smack);
			SetNextThink(0.25);
		}
		else
		{
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->time + 0.25;
			SetThink(&CCrowbar::Smack);
			SetNextThink(0.1);
		}
	}
	return fDidHit;
}


#else

int CCrowbar::Swing(int fFirst)
{
	int fDidHit = FALSE;

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}

	if (tr.flFraction >= 1.0)
	{
		if (fFirst)
		{
			// miss
			switch ((m_iSwing++) % 3)
			{
			case 0:
				SendWeaponAnim(CROWBAR_ATTACK1MISS); break;
			case 1:
				SendWeaponAnim(CROWBAR_ATTACK2MISS); break;
			case 2:
				SendWeaponAnim(CROWBAR_ATTACK3MISS); break;
			}
			m_flNextPrimaryAttack = gpGlobals->time + 0.5;
			// play wiff or swish sound
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/cbar_miss1.wav", 1, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 0xF));

			// player "shoot" animation
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		}
	}
	else
	{
		// hit
		fDidHit = TRUE;

		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		switch (((m_iSwing++) % 2) + 1)
		{
		case 0:
			SendWeaponAnim(CROWBAR_ATTACK1HIT); break;
		case 1:
			SendWeaponAnim(CROWBAR_ATTACK2HIT); break;
		case 2:
			SendWeaponAnim(CROWBAR_ATTACK3HIT); break;
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);

		ClearMultiDamage();

		if ((m_flNextPrimaryAttack + 1 < gpGlobals->time) || g_pGameRules->IsMultiplayer())
		{
			// first swing does full damage
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgCrowbar, gpGlobals->v_forward, &tr, DMG_CLUB);
		}
		else
		{
			// subsequent swings do half
			pEntity->TraceAttack(m_pPlayer->pev, gSkillData.plrDmgCrowbar / 2, gpGlobals->v_forward, &tr, DMG_CLUB);
		}
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		m_flNextPrimaryAttack = gpGlobals->time + 0.25;

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		int fHitWorld = TRUE;

		if (pEntity)
		{
			if (pEntity->Classify() != CLASS_NONE && pEntity->Classify() != CLASS_MACHINE)
			{
				// play thwack or smack sound
				switch (RANDOM_LONG(0, 2))
				{
				case 0:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/cbar_hitbod1.wav", 1, ATTN_NORM); break;
				case 1:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/cbar_hitbod2.wav", 1, ATTN_NORM); break;
				case 2:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/cbar_hitbod3.wav", 1, ATTN_NORM); break;
				}
				m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
					return TRUE;
				else
					flVol = 0.1;

				fHitWorld = FALSE;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (fHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

			if (g_pGameRules->IsMultiplayer())
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play crowbar strike
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/cbar_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			case 1:
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/cbar_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			}
		}

		// delay the decal a bit
		m_trHit = tr;
		SetThink(&CCrowbar::Smack);
		pev->nextthink = gpGlobals->time + 0.2;

		m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;
	}
	return fDidHit;
}


#endif