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

#include "weapon_mp5sd.h"
#include "weapon_layer.h"
#include "weapons/mp5sd.h"
#include "server_weapon_layer_impl.h"

LINK_ENTITY_TO_CLASS( weapon_mp5sd, CMP5SD );

CMP5SD::CMP5SD()
{
	auto layerImpl = std::make_unique<CServerWeaponLayerImpl>(this);
	auto contextImpl = std::make_unique<CMP5SDWeaponContext>(std::move(layerImpl));
	m_pWeaponContext = std::move(contextImpl);
}

void CMP5SD::Spawn( )
{
	pev->classname = MAKE_STRING(CLASSNAME_STR(MP5SD_CLASSNAME)); // hack to allow for old names
	Precache( );
	SET_MODEL( edict(), "models/weapons/w_mp5sd.mdl" );
	FallInit();// get ready to fall down.
}

void CMP5SD::Precache( void )
{
	PRECACHE_MODEL("models/weapons/v_mp5sd.mdl");
	PRECACHE_MODEL("models/weapons/w_mp5sd.mdl");
	PRECACHE_MODEL("models/weapons/p_mp5sd.mdl");
	PRECACHE_MODEL("models/shell.mdl"); // brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND("ins2/wpn/mp5sd/shoot.ogg"); //silenced handgun
}
