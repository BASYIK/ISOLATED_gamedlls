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

#include "weapon_ak74.h"
#include "weapon_layer.h"
#include "weapons/ak74.h"
#include "server_weapon_layer_impl.h"

LINK_ENTITY_TO_CLASS( weapon_ak74, CAK74 );

CAK74::CAK74()
{
	auto layerImpl = std::make_unique<CServerWeaponLayerImpl>(this);
	auto contextImpl = std::make_unique<CAK74WeaponContext>(std::move(layerImpl));
	m_pWeaponContext = std::move(contextImpl);
}

void CAK74::Spawn( )
{
	pev->classname = MAKE_STRING(CLASSNAME_STR(AK74_CLASSNAME)); // hack to allow for old names
	Precache( );
	SET_MODEL( edict(), "models/weapons/w_ak74.mdl" );
	FallInit();// get ready to fall down.
}

void CAK74::Precache( void )
{
	PRECACHE_MODEL("models/weapons/v_ak74.mdl");
	PRECACHE_MODEL("models/weapons/w_ak74.mdl");
	PRECACHE_MODEL("models/weapons/p_ak74.mdl");
	PRECACHE_MODEL("models/shell.mdl"); // brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");

	PRECACHE_SOUND("cof/guns/ak74/shoot.ogg"); //silenced handgun
}
