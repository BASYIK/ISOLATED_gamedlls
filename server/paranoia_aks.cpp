
/*******************************************************
*	weapon_aks class
*
*	(�������� ������� �����������)
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

class CAKS : public CBaseToggleWeapon
{
public:
	void	Spawn(void);
	void	Precache(void);
	int		iItemSlot(void) { return 3; }
	int		GetItemInfo(ItemInfo* p);
	int		AddToPlayer(CBasePlayer* pPlayer);
	BOOL	Deploy(void);

	void Attack1(void);
	void Attack2(void);
	void Reload1(void);
	void Reload2(void);
	void Idle1(void);
	void Idle2(void);
	int ChangeModeTo1(void);
	int ChangeModeTo2(void);
	//	Vector GetSpreadVec1( void );
	//	Vector GetSpreadVec2( void );

	int m_iShell;

private:
	unsigned short m_usAKS;
};

enum aks_e
{
	AKS_IDLE_A = 0,
	AKS_RELOAD_A,
	AKS_DRAW,
	AKS_SHOOT_A,
	AKS_IDLE_B,
	AKS_CHANGETO_B,
	AKS_CHANGETO_A,
	AKS_SHOOT_B,
	AKS_RELOAD_B,
};

LINK_ENTITY_TO_CLASS( weapon_aks, CAKS );


void CAKS::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_aks");
	Precache( );
	SET_MODEL(ENT(pev), "models/w_aks.mdl");
	m_iId = WEAPON_AKS;

	m_iDefaultAmmo = AKS_DEFAULT_GIVE;

	FallInit();
}

void CAKS::Precache( void )
{
	PRECACHE_MODEL("models/v_aks.mdl");
	PRECACHE_MODEL("models/w_aks.mdl");
	PRECACHE_MODEL("models/p_aks.mdl");

	m_iShell = PRECACHE_MODEL ("models/aks_shell.mdl");

	PRECACHE_SOUND ("weapons/ak74_fp.wav");
	PRECACHE_SOUND ("weapons/aks_fire2.wav");
	PRECACHE_SOUND ("weapons/aks_fire3.wav");

	PRECACHE_SOUND ("weapons/357_cock1.wav");

	m_flTimeWeaponIdle = 0; // fix to resend idle animation
}

int CAKS::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "ak";
	p->iMaxAmmo1 = AK_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AKS_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_AKS;
	p->iWeight = MP5_WEIGHT;

	return 1;
}

int CAKS::AddToPlayer( CBasePlayer *pPlayer )
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

BOOL CAKS::Deploy()
{
	InitToggling();
	return DefaultDeploy("models/v_aks.mdl", "models/p_aks.mdl", AKS_DRAW, "mp5", UseDecrement());
}


void CAKS::Attack1()
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
	m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, vecSpread, 8192, BULLET_PLAYER_AKS, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	SendWeaponAnim(AKS_SHOOT_A);
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/ak74_fp.wav", RANDOM_FLOAT(0.92, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));

	DefPrimPunch();
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15);
}

void CAKS::Attack2()
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

	float spread = ExpandSpread( m_pMySpread->sec_expand );
	EqualizeSpread( &spread, m_pMySpread->sec_equalize );
	Vector vecSpread = AdvanceSpread( m_pMySpread->sec_minspread, m_pMySpread->sec_addspread, spread);
	m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, vecSpread, 8192, BULLET_PLAYER_AKS, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );

	SendWeaponAnim(AKS_SHOOT_B);
	EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/ak74_fp.wav", RANDOM_FLOAT(0.92, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));

	DefSecPunch();
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
	m_flTimeWeaponIdle = gpGlobals->time + RANDOM_FLOAT(10, 15);
}

void CAKS::Reload1()
{
	DefaultReload( AKS_MAX_CLIP, AKS_RELOAD_A, 2.3 );
}

void CAKS::Reload2()
{
	DefaultReload( AKS_MAX_CLIP, AKS_RELOAD_B, 2.3 );
}

void CAKS::Idle1( void )
{
	SendWeaponAnim( AKS_IDLE_A );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;
}

void CAKS::Idle2( void )
{
	SendWeaponAnim( AKS_IDLE_B );
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;
}

int CAKS::ChangeModeTo1()
{
	SendWeaponAnim( AKS_CHANGETO_A );
	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.38;
	m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack;
	m_flTimeWeaponIdle = gpGlobals->time + 0.38 + 0.5;
	return 1;
}

int CAKS::ChangeModeTo2()
{
	SendWeaponAnim( AKS_CHANGETO_B );
	m_pPlayer->m_flNextAttack = gpGlobals->time + 0.38;
	m_flNextSecondaryAttack = m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack;
	m_flTimeWeaponIdle = gpGlobals->time + 0.38 + 0.5;
	return 1;
}







/**************************** Ammoboxes and ammoclips *********************/

class CAksAmmoClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_aksammo.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_aksammo.mdl");
		PRECACHE_SOUND ("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 30, "ak", AK_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_aks, CAksAmmoClip );

class CAksAmmoBox : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_aksammobox.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_aksammobox.mdl");
		PRECACHE_SOUND ("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( 90, "ak", AK_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_aksbox, CAksAmmoBox );
