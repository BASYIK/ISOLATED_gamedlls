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

#define WEAPON_AK74		29
#define AK74_WEIGHT		20
#define AK74_MAX_CLIP		30
#define AK74_DEFAULT_GIVE	30
#define AK74_CLASSNAME		weapon_ak74

// Animations
enum AK74Animations_e
{
	AK74_IDLE = 0,
	AK74_DRAW_FIRST,
	AK74_DRAW,
	AK74_SHOOT1,
	AK74_SHOOT2,
	AK74_SHOOT3,
	AK74_SHOOT_EMPTY,
	AK74_IRON_TO,
	AK74_IRON_FROM,
	AK74_IRON_IDLE,
	AK74_IRON_SHOOT1,
	AK74_IRON_SHOOT2,
	AK74_IRON_SHOOT3,
	AK74_IRON_SHOOT_EMPTY,
	AK74_RELOAD,
	AK74_RELOAD_EMPTY_1,
	AK74_RELOAD_EMPTY_2,
	AK74_MELEE
};


class CAK74WeaponContext : public CBaseWeaponContext
{
public:
	CAK74WeaponContext() = delete;
	CAK74WeaponContext(std::unique_ptr<IWeaponLayer> &&layer);
	~CAK74WeaponContext() = default;
	
	int iItemSlot() override { return 2; }
	int GetItemInfo(ItemInfo *p) const override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	void AK74Fire( float flSpread, float flCycleTime, bool fUseAutoAim );
	Vector GetSpreadVec(void) override { return m_iADSMode == IRON_IN ? VECTOR_CONE_1DEGREES : VECTOR_CONE_3DEGREES; }

	uint16_t m_usFireAK741;
};

template<>
struct CBaseWeaponContext::AssignedWeaponID<CAK74WeaponContext> {
	static constexpr int32_t value = WEAPON_AK74;
};
