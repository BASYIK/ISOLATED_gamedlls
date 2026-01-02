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

#pragma once
#include "weapon_context.h"
#include "weapon_layer.h"
#include <memory>

#define WEAPON_PKM		30
#define PKM_WEIGHT		20
#define PKM_MAX_CLIP		30
#define PKM_DEFAULT_GIVE	30
#define PKM_CLASSNAME		weapon_pkm

// Animations
enum PKMAnimations_e
{
	IDLE = 0,
	DRAW_FIRST,
	DRAW,
	HOLSTER,
	FIRE1,
	FIRE2,
	FIRE3,
	DRYFIRE,
	RELOAD,
	RELOAD_EMPTY,
	IRON_IDLE,
	IRON_FIRE1,
	IRON_FIRE2,
	IRON_FIRE3,
	IRON_DRYFIRE,
	IRON_TO,
	IRON_FROM,
	BIPOD_IN,
	BIPOD_OUT,
	BIPOD_IDLE,
	BIPOD_FIRE1,
	BIPOD_FIRE2,
	BIPOD_FIRE3,
	BIPOD_DRYFIRE,
	BIPOD_RELOAD,
	BIPOD_RELOAD_EMPTY,
	BIPOD_IRON_IDLE,
	BIPOD_IRON_FIRE1,
	BIPOD_IRON_FIRE2,
	BIPOD_IRON_FIRE3,
	BIPOD_IRON_DRYFIRE,
	BIPOD_IRON_TO,
	BIPOD_IRON_FROM,
	BIPOD_IRON_IN,
	BIPOD_IRON_OUT
};

enum PKM_Bodygroups
{
	ARMS = 0,
	PKM_STUDIO0,
	PKM_STUDIO1,
	PKM_STUDIO2,
	MAIN_BULLETS,
	HELPER_BULLETS
};

class CPKMWeaponContext : public CBaseWeaponContext
{
public:
	CPKMWeaponContext() = delete;
	CPKMWeaponContext(std::unique_ptr<IWeaponLayer> &&layer);
	~CPKMWeaponContext() = default;
	
	int iItemSlot() override { return 2; }
	int GetItemInfo(ItemInfo *p) const override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	void PKMFire( float flSpread, float flCycleTime, bool fUseAutoAim );
	Vector GetSpreadVec(void) override { return m_iADSMode == IRON_IN ? VECTOR_CONE_1DEGREES : VECTOR_CONE_3DEGREES; }

	uint16_t m_usFirePKM1;
};

template<>
struct CBaseWeaponContext::AssignedWeaponID<CPKMWeaponContext> {
	static constexpr int32_t value = WEAPON_PKM;
};
