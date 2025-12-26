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

#define WEAPON_GLOCK		2
#define GLOCK_WEIGHT		10
#define GLOCK_MAX_CLIP		17
#define GLOCK_DEFAULT_GIVE	17
#define GLOCK_CLASSNAME		weapon_9mmhandgun

// Animations
enum glock17e
{
	IDLE = 0,
	IDLE_EMPTY,
	DRAW_FIRST,
	DRAW,
	DRAW_EMPTY,
	HOLSTER,
	HOLSTER_EMPTY,
	FIRE1,
	FIRE2,
	FIRE3,
	FIRE_LAST,
	DRYFIRE,
	RELOAD,
	RELOAD_EMPTY,
	RELOAD_EMPTY_2,
	IRON_IDLE,
	IRON_IDLE_EMPTY,
	IRON_FIRE1,
	IRON_FIRE2,
	IRON_FIRE3,
	IRON_FIRE4,
	IRON_FIRE_LAST,
	IRON_DRYFIRE,
	IRON_TO,
	IRON_TO_EMPTY,
	IRON_FROM,
	IRON_FROM_EMPTY
};


class CGlockWeaponContext : public CBaseWeaponContext
{
public:
	CGlockWeaponContext() = delete;
	CGlockWeaponContext(std::unique_ptr<IWeaponLayer> &&layer);
	~CGlockWeaponContext() = default;
	
	int iItemSlot() override { return 2; }
	int GetItemInfo(ItemInfo *p) const override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	void GlockFire( float flSpread, float flCycleTime, bool fUseAutoAim );

	uint16_t m_usFireGlock1;
	uint16_t m_usFireGlock2;
};

template<>
struct CBaseWeaponContext::AssignedWeaponID<CGlockWeaponContext> {
	static constexpr int32_t value = WEAPON_GLOCK;
};
