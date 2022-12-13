
/*******************************************************
*	CBaseToggleWeapon and CBaseSpreadWeapon class declaration.
*
*	look paranoia_wpn.cpp for implementation
*
*	written by BUzer
*******************************************************/


enum spread_equalize
{
	E_LINEAR = 0,	// изменение разброса линейно во времени
	E_QUAD,		// по параболе (вначале узкий, потом резко расширяется)
	E_CUBE,		// кубическая парабола
	E_SQRT,		// наоборот (быстро расширяется и плавно переходит в максимальный)
};

typedef struct spreadparams_s {
	char szWeaponName[32];

	Vector			pri_minspread;
	Vector			pri_addspread;
	spread_equalize	pri_equalize;
	float			pri_expand;
	Vector			pri_minpunch;
	Vector			pri_maxpunch;

	Vector			sec_minspread;
	Vector			sec_addspread;
	spread_equalize	sec_equalize;
	float			sec_expand;
	Vector			sec_minpunch;
	Vector			sec_maxpunch;

	float	pri_speed;
	float	sec_speed;

	int		pri_jump;
	int		sec_jump;

	float		returntime;
} spreadparams_t;


class CBaseSpreadWeapon : public CBasePlayerWeapon
{
	DECLARE_CLASS(CBaseSpreadWeapon, CBasePlayerWeapon);
public:
	DECLARE_DATADESC();
	// weapons should call this from their Spawn() func
	void InitSpread (float time);

	// weapons should call this from functions, who need m_pMySpread pointer
	void SetTablePointer( void );

	// weapons should call this from their attack functions
	float ExpandSpread( float expandPower );

	virtual void PlayerJump();

	spreadparams_t	*m_pMySpread; // pointer to global spread table for this gun

	void DefPrimPunch();
	void DefSecPunch();

	float	CalcSpread( void );
	virtual Vector	GetSpreadVec( void );

	float m_flSpreadTime; // time to return from full spread
	float m_flLastShotTime;
	float m_flLastSpreadPower; // [0..1] range value


	virtual BOOL UseDecrement(void)
	{
		return TRUE;
	}
};

Vector	AdvanceSpread( Vector &baseSpread, Vector &addSpread, float spread );
void	EqualizeSpread( float *spread, spread_equalize type );


enum weapon_mode
{
	MODE_A = 0,
	MODE_B,
};

class CBaseToggleWeapon : public CBaseSpreadWeapon
{
	DECLARE_CLASS(CBaseToggleWeapon, CBaseSpreadWeapon);
public:
	DECLARE_DATADESC();
	// weapons should redefine this funcs
	virtual void Attack1( void ) {};
	virtual void Attack2( void ) {};
	virtual void Reload1( void ) {};
	virtual void Reload2( void ) {};
	virtual void Idle1( void ) {}; //	weapons only should send animation and
	virtual void Idle2( void ) {}; //		set next time
	virtual int ChangeModeTo1( void ) {return 0;};
	virtual int ChangeModeTo2( void ) {return 0;};
	virtual Vector GetSpreadVec1( void );
	virtual Vector GetSpreadVec2( void );

	// weapons should call this from Deploy() func
	void	InitToggling( void );


	virtual void PrimaryAttack( void );				// do "+ATTACK"
	virtual void SecondaryAttack( void );			// do "+ATTACK2"
	virtual void Reload( void );					// do "+RELOAD"
	virtual void WeaponIdle( void );
	virtual void Holster( int skiplocal );
	virtual Vector GetSpreadVec( void );

	virtual int GetMode(void)
	{
		switch (m_iWeaponMode)
		{
		default:
		case MODE_A:
			return 1;
		case MODE_B:
			return 2;
		}
		return 0;
	}

	int		m_iWeaponMode;


	virtual BOOL UseDecrement(void)
	{
		return TRUE;
	}
};
/*
class CAPS : public CBaseToggleWeapon
{
public:
	void	Spawn(void);
	void	Precache(void);
	int		iItemSlot(void) { return 2; }
	int		GetItemInfo(ItemInfo *p);
	int		AddToPlayer(CBasePlayer *pPlayer);
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
	unsigned short m_usAPS;
	unsigned short m_usAPS2;

};
*/
/*


// buz: class declaration moved here
class CMP5 : public CBaseToggleWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int iItemSlot(void) { return 3; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer(CBasePlayer *pPlayer);

	// Wargon: Фикс невозможности использовать MP5, если у него остались только подствольные гранаты.
	BOOL IsUseable(void);

	void PrimaryAttack(void);
	void SecondaryAttack(void);
	int SecondaryAmmoIndex(void);
	BOOL Deploy(void);
	void Reload(void);
	void WeaponIdle();
	void Idle1(void);
#ifndef CLIENT_DLL
	int		Save(CSave &save);
	int		Restore(CRestore &restore);
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	//	float m_flNextAnimTime;
	int m_iShell;
	float m_flLoadGrenadeTime;
	int m_iGrenadeLoaded;

private:
	unsigned short m_usMP5;
	unsigned short m_usMP52;
};


class CAK47 : public CBaseToggleWeapon
{
public:
	void	Spawn(void);
	void	Precache(void);
	int		iItemSlot(void) { return 3; }
	int		GetItemInfo(ItemInfo *p);
	int		AddToPlayer(CBasePlayer *pPlayer);
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
	unsigned short m_usAK47;
};

class CGroza : public CBaseToggleWeapon
{
public:
	void	Spawn(void);
	void	Precache(void);
	int		iItemSlot(void) { return 4; }
	int		GetItemInfo(ItemInfo *p);
	int		AddToPlayer(CBasePlayer *pPlayer);
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


class CRPK : public CBaseToggleWeapon
{
public:
	void	Spawn(void);
	void	Precache(void);
	int		iItemSlot(void) { return 4; }
	int		GetItemInfo(ItemInfo *p);
	int		AddToPlayer(CBasePlayer *pPlayer);
	BOOL	Deploy(void);

	void Attack1(void);
	void Attack2(void);
	void Reload1(void);
	void Reload2(void);
	void Idle1(void);
	void Idle2(void);
	int ChangeModeTo1();
	int ChangeModeTo2();

	int m_iShell;

private:
	unsigned short m_usRPK;
};

class CSVD : public CBaseToggleWeapon
{
public:
	void	Spawn(void);
	void	Precache(void);
	int		iItemSlot(void) { return 4; }
	int		GetItemInfo(ItemInfo *p);
	int		AddToPlayer(CBasePlayer *pPlayer);
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
	unsigned short m_usSVD;
};


class CVAL : public CBaseToggleWeapon
{
public:
	void	Spawn(void);
	void	Precache(void);
	int		iItemSlot(void) { return 4; }
	int		GetItemInfo(ItemInfo *p);
	int		AddToPlayer(CBasePlayer *pPlayer);
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
	unsigned short m_usVAL;
};
*/