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

#define WEAPON_MP5SD		25
#define MP5SD_WEIGHT		15
#define MP5SD_MAX_CLIP		30
#define MP5SD_DEFAULT_GIVE	30
#define MP5SD_CLASSNAME		weapon_mp5sd

class CMP5SDWeaponContext : public CBaseWeaponContext
{
public:
	CMP5SDWeaponContext() = delete;
	CMP5SDWeaponContext(std::unique_ptr<IWeaponLayer> &&layer);
	~CMP5SDWeaponContext() = default;
	
	int iItemSlot() override { return 4; }
	int GetItemInfo(ItemInfo *p) const override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	void MP5SDFire( float flSpread, float flCycleTime, bool fUseAutoAim );
	Vector GetSpreadVec(void) override { return m_iADSMode == IRON_IN ? VECTOR_CONE_1DEGREES : VECTOR_CONE_2DEGREES; }

	uint16_t m_usFireMP5SD1;
};

template<>
struct CBaseWeaponContext::AssignedWeaponID<CMP5SDWeaponContext> {
	static constexpr int32_t value = WEAPON_MP5SD;
};
