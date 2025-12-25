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

#ifndef GAME_H
#define GAME_H
#include <unordered_map>
#include <string>
#include "HashMap.h"

extern void GameDLLInit( void );
extern void GameDLLShutdown( void );

extern cvar_t	displaysoundlist;

// multiplayer server rules
extern cvar_t	teamplay;
extern cvar_t	fraglimit;
extern cvar_t	timelimit;
extern cvar_t	friendlyfire;
extern cvar_t	falldamage;
extern cvar_t	weaponstay;
extern cvar_t	forcerespawn;
extern cvar_t	flashlight;
extern cvar_t	aimcrosshair;
extern cvar_t	decalfrequency;
extern cvar_t	teamlist;
extern cvar_t	teamoverride;
extern cvar_t	defaultteam;
extern cvar_t	allowmonsters;

// Engine Cvars
extern cvar_t	*g_psv_gravity;
extern cvar_t	*g_psv_aim;
extern cvar_t	*g_psv_stepsize;
extern cvar_t	*g_footsteps;
extern cvar_t	*g_debugdraw;	// novodex physics debug
extern cvar_t	*p_speeds;
extern cvar_t	*g_physdebug;	// quake physics debug
extern cvar_t	*g_allow_physx;
extern cvar_t	g_sync_physic;

struct player_score_t {
	float frags;
	float multiplier;
	int deaths;
};

// maps a steam ID to their score, for preserving scores across level changes and disconnects
extern std::unordered_map<const char *, player_score_t> g_playerScores;
extern std::unordered_map<const char *, player_score_t> g_oldPlayerScores; // state on level load, used in case of map restarts

struct player_inventory_t {
	StringSet weapons;
	int weaponClips[MAX_WEAPONS];
	int	m_rgAmmo[MAX_AMMO_SLOTS];
	int activeWeaponId;
	float health;
	float armor;
	int flashlightBattery;
	bool hasLongjump;
};

// inventory to keep across map changes
extern std::unordered_map<const char *, player_inventory_t> g_playerInventory;
extern bool g_clearInventoriesNextMap; // true if player inventories should be cleared on the next map

#endif		// GAME_H

