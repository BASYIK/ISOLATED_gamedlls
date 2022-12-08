
/*******************************************************
*	weapon_groza class
*
*	written by BUzer for Half-Life:Paranoia modification
*******************************************************/

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

class CGroza : public CBaseToggleWeapon
{
public:
	void	Spawn(void);
	void	Precache(void);
	int		iItemSlot(void) { return 4; }
	int		GetItemInfo(ItemInfo* p);
	int		AddToPlayer(CBasePlayer* pPlayer);
	BOOL	Deploy(void);
	void	Holster(int skiplocal);

	void Attack1(void);
	void Attack2(void);
	void Reload1(void);
	void Reload2(void);
	void Idle1(void);
	void Idle2(void);
	int ChangeModeTo1(void);
	int ChangeModeTo2(void);
	//	Vector GetSpreadVec1( void );
	Vector GetSpreadVec2(void);
	int	GetMode(void);

	int m_iShell;

private:
	unsigned short m_usGroza;
};


enum groza_e
{
	GROZA_IDLE = 0,
	GROZA_DRAW,
	GROZA_SHOOT,
	GROZA_RELOAD,
};

LINK_ENTITY_TO_CLASS( weapon_groza, CGroza );


void CGroza::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_groza");
	Precache( );
	SET_MODEL(ENT(pev), "models/w_groza.mdl");
	m_iId = WEAPON_GROZA;

	m_iDefaultAmmo = GROZA_DEFAULT_GIVE;

	FallInit();
}

void CGroza::Precache( void )
{
	PRECACHE_MODEL("models/v_groza.mdl");
	PRECACHE_MODEL("models/w_groza.mdl");
	PRECACHE_MODEL("models/p_groza.mdl");

	m_iShell = PRECACHE_MODEL ("models/groza_shell.mdl");

	PRECACHE_SOUND ("weapons/groza_fire.wav");
	PRECACHE_SOUND ("weapons/groza_fire2.wav");
	PRECACHE_SOUND ("weapons/groza_fire3.wav");

	PRECACHE_SOUND ("weapons/357_cock1.wav");

	m_usGroza = PRECACHE_EVENT( 1, "events/groza.sc" );

	m_flTimeWeaponIdle = 0; // fix to resend idle animation
}

int CGroza::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "groza";
	p->iMaxAmmo1 = AK_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GROZA_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_GROZA;
	p->iWeight = MP5_WEIGHT;

	return 1;
}

int CGroza::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CGroza::Deploy()
{
	InitToggling();
	return DefaultDeploy("models/v_groza.mdl", "models/p_groza.mdl", GROZA_DRAW, "mp5", UseDecrement());
}

Vector CGroza::GetSpreadVec2( void )
{
	return VECTOR_CONE_2DEGREES;
}

void CGroza::Attack1()
{
	// don't fire underwater
	if ((m_iClip <= 0) || (m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD))
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_iClip--;
	
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	float spread = ExpandSpread( m_pMySpread->pri_expand );
	EqualizeSpread( &spread, m_pMySpread->pri_equalize );
	Vector vecSpread = AdvanceSpread( m_pMySpread->pri_minspread, m_pMySpread->pri_addspread, spread);
	m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, vecSpread, 8192, BULLET_PLAYER_GROZA, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	SendWeaponAnim(GROZA_SHOOT);
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/groza_fire.wav", RANDOM_FLOAT(0.92, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));

	DefPrimPunch();
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15);
}

void CGroza::Attack2()
{
	// don't fire underwater
	if ((m_iClip <= 0) || (m_pPlayer->pev->waterlevel == 3 && m_pPlayer->pev->watertype > CONTENT_FLYFIELD))
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	m_iClip--;
	
	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

//	float spread = ExpandSpread( 0.2 );
//	Vector vecSpread = AdvanceSpread( VECTOR_CONE_2DEGREES, VECTOR_CONE_4DEGREES, spread);
	Vector vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_2DEGREES, 8192, BULLET_PLAYER_GROZA, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	SendWeaponAnim(GROZA_SHOOT);
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/groza_fire.wav", RANDOM_FLOAT(0.92, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));

	DefSecPunch();
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15);
}

void CGroza::Reload1()
{
	DefaultReload( GROZA_MAX_CLIP, GROZA_RELOAD, 2.3 );
}

void CGroza::Reload2()
{
	if (DefaultReload( GROZA_MAX_CLIP, GROZA_RELOAD, 2.3 ))
	{
		CBaseToggleWeapon::SecondaryAttack(); // remove scope
		SetNextAttack(SequenceDuration());
	}
}

void CGroza::Idle1( void )
{
	SendWeaponAnim( GROZA_IDLE );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;
}

void CGroza::Idle2( void )
{
	SendWeaponAnim( GROZA_IDLE );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;
}

int CGroza::ChangeModeTo1()
{
	m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;

	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.3;
	m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack;
	m_flTimeWeaponIdle = gpGlobals->time + 0.3 + 0.5;
//	ALERT(at_console, "val to 0\n");
	return 1;
}

int CGroza::ChangeModeTo2()
{
	if (m_pPlayer->m_iGasMaskOn)
	{
#ifndef CLIENT_DLL
		UTIL_ShowMessage("#GAS_AND_SCOPE", m_pPlayer );
#endif

		m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack;
		m_flTimeWeaponIdle = gpGlobals->time + 0.5 + 0.5;
		return 0;
	//	m_pPlayer->ToggleGasMask();
	}
	if (m_pPlayer->m_iHeadShieldOn)
	{
#ifndef CLIENT_DLL

		UTIL_ShowMessage("#SCOPE_AND_SHIELD", m_pPlayer );

#endif

		m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
		m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack;
		m_flTimeWeaponIdle = gpGlobals->time + 0.5 + 0.5;
		return 0;
	//	m_pPlayer->ToggleHeadShield();
	}

//	m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 30;

	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.5;
	m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack;
	m_flTimeWeaponIdle = gpGlobals->time + 0.5 + 0.5;
//	ALERT(at_console, "val to 30\n");
	return 1;
}

void CGroza::Holster( int skiplocal )
{
	m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0; // reset fov
	CBaseToggleWeapon::Holster(skiplocal);
}

int CGroza::GetMode( void )
{
	switch (m_iWeaponMode)
	{
	default:
	case MODE_A:
		return 1;

	case MODE_B:
		return 3; // hack to draw gun-specific scope on client
	}
}





/**************************** Ammoboxes and ammoclips *********************/

class CGrozaAmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_grammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_grammo.mdl");
		PRECACHE_SOUND ("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 30, "groza", AK_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_groza, CGrozaAmmoClip );


class CGrozaAmmoBox : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_grammobox.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_grammobox.mdl");
		PRECACHE_SOUND ("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 80, "groza", AK_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_grozabox, CGrozaAmmoBox );
