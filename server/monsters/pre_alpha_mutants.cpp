/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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

//=========================================================
// Mutant.cpp
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"player.h"
#include	"decals.h"
#include	"animation.h"
#include	"studio.h"

/* Whether mutant locks the player during the bite attack.
 * This may have undesired side-effects, e.g. in combination with trigger_playerfreeze or trigger_camera,
 * since both mutant and the trigger use the same technique to lock the player.
 */
#define FEATURE_MUTANT_LOCK_PLAYER 1

#define		MUTANT_MELEE_ATTACK_RADIUS		70

enum
{
	TASK_MUTANT_GET_PATH_TO_ENEMY_CORPSE = LAST_COMMON_TASK + 1
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================

#define MUTANT_AE_SLASH_RIGHT	( 1 )
#define MUTANT_AE_SLASH_LEFT	( 2 )
#define MUTANT_AE_SPIT			( 3 )
#define MUTANT_AE_THROW		( 4 )

#define MUTANT_AE_BITE1			( 19 )
#define MUTANT_AE_BITE2			( 20 )
#define MUTANT_AE_BITE3			( 21 )
#define MUTANT_AE_BITE4			( 22 )

#define MUTANT_SCRIPT_EVENT_SOUND ( 1011 )

//=========================================================
// Mutant's guts projectile
//=========================================================
class CMutantGuts : public CSquidSpit
{
public:
	void Spawn(void);
	void Touch(CBaseEntity *pOther);
};

void CMutantGuts::Spawn()
{
	pev->movetype = MOVETYPE_FLY;
	pev->classname = MAKE_STRING( "mutantguts" );

	pev->solid = SOLID_BBOX;
	pev->rendermode = kRenderTransAlpha;
	pev->renderamt = 255;

	SET_MODEL( ENT( pev ), "sprites/bigspit.spr" );
	pev->frame = 0;
	pev->scale = 0.5;
	pev->rendercolor.x = 255;

	UTIL_SetSize( pev, Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );

	m_maxFrame = (float)MODEL_FRAMES( pev->modelindex ) - 1;
}

void CMutantGuts::Touch( CBaseEntity *pOther )
{
	TraceResult tr;
	int iPitch;

	// splat sound
	iPitch = RANDOM_FLOAT( 90, 110 );

	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, "bullchicken/bc_acid1.wav", 1, ATTN_NORM, 0, iPitch );

	switch( RANDOM_LONG( 0, 1 ) )
	{
	case 0:
		EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "bullchicken/bc_spithit1.wav", 1, ATTN_NORM, 0, iPitch );
		break;
	case 1:
		EMIT_SOUND_DYN( ENT( pev ), CHAN_WEAPON, "bullchicken/bc_spithit2.wav", 1, ATTN_NORM, 0, iPitch );
		break;
	}

	if( !pOther->pev->takedamage )
	{
		// make a splat on the wall
		UTIL_TraceLine( pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, ENT( pev ), &tr );
		UTIL_BloodDecalTrace( &tr, BLOOD_COLOR_RED );
		UTIL_BloodDrips( tr.vecEndPos, UTIL_RandomBloodVector(), BLOOD_COLOR_RED, 35 );
	}
	else
	{
		pOther->TakeDamage( pev, pev, 25, DMG_GENERIC );
	}

	SetThink( &CBaseEntity::SUB_Remove );
	pev->nextthink = gpGlobals->time;
}

//=========================================================
// CMutant
//=========================================================
class CMutant : public CBaseMonster
{
	DECLARE_CLASS(CMutant,CBaseMonster)
public:
	DECLARE_DATADESC();
	void Spawn(void);
	void Precache(void);

	int  Classify(void);
	void SetYawSpeed();
	void HandleAnimEvent(MonsterEvent_t *pEvent);
	int IgnoreConditions();
	void IdleSound( void );
	void PainSound( void );
	void DeathSound( void );
	void AlertSound( void );
	void StartTask(Task_t *pTask);

	BOOL CheckMeleeAttack2(float flDot, float flDist);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	void SetActivity( Activity NewActivity );

	Schedule_t *GetSchedule();
	Schedule_t *GetScheduleOfType( int Type );
	void RunTask(Task_t* pTask);

	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
	void Killed(entvars_t *pevAttacker, int iGib);

	void UnlockPlayer();
	CMutantGuts* GetMutantGuts(const Vector& pos);
	void ClearGuts();

	CUSTOM_SCHEDULES

	static const char* pPainSounds[];
	static const char* pIdleSounds[];
	static const char* pDeathSounds[];
	static const char *pAttackHitSounds[];
	static const char *pAttackMissSounds[];

	//virtual int SizeForGrapple() { return GRAPPLE_LARGE; }
protected:
	float m_flNextFlinch;
	float m_flNextThrowTime;// last time the mutant used the guts attack.
	CMutantGuts* m_pMutantGuts;
#if FEATURE_MUTANT_LOCK_PLAYER
	BOOL m_fPlayerLocked;
	EHANDLE m_lockedPlayer;
#endif
	bool m_meleeAttack2;
	bool m_playedAttackSound;
};

LINK_ENTITY_TO_CLASS(monster_mutant, CMutant)

const char* CMutant::pPainSounds[] = {
	"mutant/mutant_pain1.wav",
	"mutant/mutant_pain2.wav",
	"mutant/mutant_pain3.wav"
};

const char* CMutant::pIdleSounds[] = {
	"mutant/mutant_idle1.wav",
	"mutant/mutant_idle2.wav",
	"mutant/mutant_idle3.wav"
};

const char* CMutant::pDeathSounds[] = {
	"mutant/mutant_pain1.wav",
	"mutant/mutant_pain2.wav",
	"mutant/mutant_pain3.wav"
};

const char* CMutant::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CMutant::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

BEGIN_DATADESC(CMutant)
DEFINE_FIELD(m_flNextFlinch, FIELD_TIME),
DEFINE_FIELD(m_flNextThrowTime, FIELD_TIME),
#if FEATURE_MUTANT_LOCK_PLAYER
DEFINE_FIELD(m_fPlayerLocked, FIELD_BOOLEAN),
#endif
END_DATADESC();

void CMutant::Killed(entvars_t *pevAttacker, int iGib)
{
	ClearGuts();
	UnlockPlayer();
	CBaseMonster::Killed(pevAttacker, iGib);
}

void CMutant::UnlockPlayer()
{
#if FEATURE_MUTANT_LOCK_PLAYER
	if (m_fPlayerLocked)
	{
		CBasePlayer* player = 0;
		if (m_lockedPlayer != 0 && m_lockedPlayer->IsPlayer())
			player = (CBasePlayer*)((CBaseEntity*)m_lockedPlayer);
		else // if ehandle is empty for some reason just unlock the first player
			player = (CBasePlayer*)UTIL_FindEntityByClassname(0, "player");

		if (player)
			player->EnableControl(TRUE);

		m_lockedPlayer = 0;
		m_fPlayerLocked = FALSE;
	}
#endif
}

CMutantGuts* CMutant::GetMutantGuts(const Vector &pos)
{
	if (m_pMutantGuts)
		return m_pMutantGuts;
	CMutantGuts *pGuts = GetClassPtr( (CMutantGuts *)NULL );
	pGuts->Spawn();

	UTIL_SetOrigin( pGuts, pos );

	m_pMutantGuts = pGuts;
	return m_pMutantGuts;
}

void CMutant::ClearGuts()
{
	if (m_pMutantGuts)
	{
		UTIL_Remove(m_pMutantGuts);
		m_pMutantGuts = 0;
	}
}

void CMutant::PainSound( void )
{
	int pitch = 95 + RANDOM_LONG( 0, 9 );

	if( RANDOM_LONG( 0, 5 ) < 2 )
		EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch );
}

void CMutant::DeathSound( void )
{
	int pitch = 95 + RANDOM_LONG( 0, 9 );

	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_NORM, 0, pitch );
}

void CMutant::IdleSound( void )
{
	int pitch = 95 + RANDOM_LONG( 0, 9 );

	// Play a random idle sound
	EMIT_SOUND_DYN( ENT( pev ), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1.0, ATTN_NORM, 0, pitch );
}

void CMutant::AlertSound( void )
{
	const int iPitch = RANDOM_LONG(0, 9) + 95;
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1, ATTN_NORM, 0, iPitch);
}

void CMutant::SetActivity( Activity NewActivity )
{
	Activity OldActivity = m_Activity;
	int iSequence = ACTIVITY_NOT_AVAILABLE;

	if (NewActivity != ACT_RANGE_ATTACK1)
	{
		ClearGuts();
	}
	if (NewActivity == ACT_MELEE_ATTACK1 && m_hEnemy != 0)
	{
		// special melee animations
		if ((pev->origin - m_hEnemy->pev->origin).Length2D() >= 48 )
		{
			m_meleeAttack2 = false;
			iSequence = LookupSequence("attack1");
		}
		else
		{
			m_meleeAttack2 = true;
			iSequence = LookupSequence("attack2");
		}
	}
	else
	{
		UnlockPlayer();

		if (NewActivity == ACT_RUN && m_hEnemy != 0)
		{
			// special run animations
			if ((pev->origin - m_hEnemy->pev->origin).Length2D() <= 512 )
			{
				iSequence = LookupSequence("runshort");
			}
			else
			{
				iSequence = LookupSequence("runlong");
			}
		}
		else
		{
			iSequence = LookupActivity(NewActivity);
		}
	}

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

	// In case someone calls this with something other than the ideal activity
	m_IdealActivity = m_Activity;

	// Set to the desired anim, or default anim if the desired is not present
	if( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		if( pev->sequence != iSequence || !m_fSequenceLoops )
		{
			// don't reset frame between walk and run
			if( !( OldActivity == ACT_WALK || OldActivity == ACT_RUN ) || !( NewActivity == ACT_WALK || NewActivity == ACT_RUN ) )
				pev->frame = 0;
		}

		pev->sequence = iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo();
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT( at_aiconsole, "%s has no sequence for act:%d\n", STRING( pev->classname ), NewActivity );
		pev->sequence = 0;	// Set to the reset anim (if it's there)
	}
}

//=========================================================
// Classify - indicates this monster's place in the
// relationship table.
//=========================================================
int	CMutant::Classify(void)
{
	return	CLASS_ALIEN_MONSTER;
}

//=========================================================
// TakeDamage - overridden for mutant so we can keep track
// of how much time has passed since it was last injured
//=========================================================
int CMutant::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	// Take 15% damage from bullets
	if( bitsDamageType == DMG_BULLET )
	{
		Vector vecDir = pev->origin - (pevInflictor->absmin + pevInflictor->absmax) * 0.5;
		vecDir = vecDir.Normalize();
		float flForce = DamageForce( flDamage );
		pev->velocity = pev->velocity + vecDir * flForce;
		flDamage *= 0.15;
	}

	// HACK HACK -- until we fix this.
	if( IsAlive() )
		PainSound();
	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}


//=========================================================
// CheckRangeAttack1
//=========================================================
BOOL CMutant::CheckRangeAttack1(float flDot, float flDist)
{
	if (flDist < 256)
		return FALSE;

	if (IsMoving() && flDist >= 512)
	{
		// squid will far too far behind if he stops running to spit at this distance from the enemy.
		return FALSE;
	}

	if (flDist > 64 && flDist <= 784 && flDot >= 0.5 && gpGlobals->time >= m_flNextThrowTime)
	{
		if (m_hEnemy != 0)
		{
			if (fabs(pev->origin.z - m_hEnemy->pev->origin.z) > 256)
			{
				// don't try to spit at someone up really high or down really low.
				return FALSE;
			}
		}

		if (IsMoving())
		{
			// don't spit again for a long time, resume chasing enemy.
			m_flNextThrowTime = gpGlobals->time + 5;
		}
		else
		{
			// not moving, so spit again pretty soon.
			m_flNextThrowTime = gpGlobals->time + 0.5;
		}

		return TRUE;
	}

	return FALSE;
}

//=========================================================
// CheckMeleeAttack2 - both mutant's melee attacks are ACT_MELEE_ATTACK1
//=========================================================
BOOL CMutant::CheckMeleeAttack2(float flDot, float flDist)
{
	return FALSE;
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CMutant::SetYawSpeed( void )
{
	pev->yaw_speed = 120;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CMutant::HandleAnimEvent(MonsterEvent_t *pEvent)
{
	switch (pEvent->event)
	{
	case MUTANT_SCRIPT_EVENT_SOUND:
		if (m_Activity != ACT_MELEE_ATTACK1)
			EMIT_SOUND(ENT(pev), CHAN_BODY, pEvent->options, 1, ATTN_NORM);
		break;
	case MUTANT_AE_SPIT:
	{
		Vector vecArmPos, vecArmAng;
		GetAttachment(0, vecArmPos, vecArmAng);

		if (GetMutantGuts(vecArmPos))
		{
			m_pMutantGuts->pev->skin = entindex();
			m_pMutantGuts->pev->body = 1;
			m_pMutantGuts->pev->aiment = ENT(pev);
			m_pMutantGuts->pev->movetype = MOVETYPE_FOLLOW;
		}
		UTIL_BloodDrips( vecArmPos, UTIL_RandomBloodVector(), BLOOD_COLOR_RED, 35 );
	}
	break;
	case MUTANT_AE_THROW:
	{
		UTIL_MakeVectors(pev->angles);
		Vector vecArmPos, vecArmAng;
		GetAttachment(0, vecArmPos, vecArmAng);

		if (GetMutantGuts(vecArmPos))
		{
			Vector	vecSpitDir;

			Vector vecEnemyPosition;
			if (m_hEnemy != 0)
				vecEnemyPosition = (m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs);
			else
				vecEnemyPosition = m_vecEnemyLKP;
			vecSpitDir = (vecEnemyPosition - vecArmPos).Normalize();

			vecSpitDir.x += RANDOM_FLOAT(-0.05, 0.05);
			vecSpitDir.y += RANDOM_FLOAT(-0.05, 0.05);
			vecSpitDir.z += RANDOM_FLOAT(-0.05, 0);

			m_pMutantGuts->pev->body = 0;
			m_pMutantGuts->pev->skin = 0;
			m_pMutantGuts->pev->owner = ENT( pev );
			m_pMutantGuts->pev->aiment = 0;
			m_pMutantGuts->pev->movetype = MOVETYPE_FLY;
			m_pMutantGuts->pev->velocity = vecSpitDir * 900;
			m_pMutantGuts->SetThink( &CMutantGuts::Animate );
			m_pMutantGuts->pev->nextthink = gpGlobals->time + 0.1;
			UTIL_SetOrigin(m_pMutantGuts, vecArmPos);

			m_pMutantGuts = 0;
		}
		UTIL_BloodDrips( vecArmPos, UTIL_RandomBloodVector(), BLOOD_COLOR_RED, 35 );
	}
	break;

	case MUTANT_AE_SLASH_LEFT:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack(MUTANT_MELEE_ATTACK_RADIUS, 60, DMG_SLASH);
		if (pHurt)
		{
			if (FBitSet(pHurt->pev->flags, FL_MONSTER|FL_CLIENT))
			{
				pHurt->pev->punchangle.z = 9;
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * 25;
			}
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5));
		}
		else
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5));
		}
	}
	break;

	case MUTANT_AE_SLASH_RIGHT:
	{
		CBaseEntity *pHurt = CheckTraceHullAttack(MUTANT_MELEE_ATTACK_RADIUS, 60, DMG_SLASH);
		if (pHurt)
		{
			if (FBitSet(pHurt->pev->flags, FL_MONSTER|FL_CLIENT))
			{
				pHurt->pev->punchangle.z = -9;
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * -25;
			}
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5));
		}
		else
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1, ATTN_NORM, 0, 100 + RANDOM_LONG(-5,5));
		}
	}
	break;

	case MUTANT_AE_BITE1:
	case MUTANT_AE_BITE2:
	case MUTANT_AE_BITE3:
	case MUTANT_AE_BITE4:
		{
			int iPitch;
			CBaseEntity *pHurt = CheckTraceHullAttack(MUTANT_MELEE_ATTACK_RADIUS, 60, DMG_SLASH);

			if (pHurt)
			{
				// croonchy bite sound
				iPitch = RANDOM_FLOAT(90, 110);
				switch (RANDOM_LONG(0, 1))
				{
				case 0:
					EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "bullchicken/bc_bite2.wav", 1, ATTN_NORM, 0, iPitch);
					break;
				case 1:
					EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "bullchicken/bc_bite3.wav", 1, ATTN_NORM, 0, iPitch);
					break;
				}

				if (FBitSet(pHurt->pev->flags, FL_MONSTER|FL_CLIENT))
				{
					if (pEvent->event == MUTANT_AE_BITE4)
					{
						pHurt->pev->punchangle.x = 15;
						pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_forward * 75;
					}
					else
					{
						pHurt->pev->punchangle.x = 9;
						pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_forward * 25;
					}
				}
#if FEATURE_MUTANT_LOCK_PLAYER
				if (pEvent->event == MUTANT_AE_BITE4)
				{
					UnlockPlayer();
				}
				else if (pHurt->IsPlayer() && pHurt->IsAlive())
				{
					if (!m_fPlayerLocked)
					{
						CBasePlayer* player = (CBasePlayer*)pHurt;
						player->EnableControl(FALSE);
						m_lockedPlayer = player;
						m_fPlayerLocked = TRUE;
					}
				}
#endif
			}
		}
		break;

	default:
		CBaseMonster::HandleAnimEvent(pEvent);
	}
}

#define MUTANT_FLINCH_DELAY 2

int CMutant::IgnoreConditions( void )
{
	int iIgnore = CBaseMonster::IgnoreConditions();

	if (m_Activity == ACT_RANGE_ATTACK1)
	{
		iIgnore |= bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE | bits_COND_ENEMY_TOOFAR | bits_COND_ENEMY_OCCLUDED;
	}
	else if( m_Activity == ACT_MELEE_ATTACK1 )
	{
		if( m_flNextFlinch >= gpGlobals->time )
			iIgnore |= ( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE );
	}

	if( ( m_Activity == ACT_SMALL_FLINCH ) || ( m_Activity == ACT_BIG_FLINCH ) )
	{
		if( m_flNextFlinch < gpGlobals->time )
			m_flNextFlinch = gpGlobals->time + MUTANT_FLINCH_DELAY;
	}

	return iIgnore;
}

//=========================================================
// Spawn
//=========================================================
void CMutant::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/z_mutant_alpha.mdl");
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->effects = 0;
	pev->health = gSkillData.zombieHealth + 300;
	m_flFieldOfView = 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_DOORS_GROUP;

	m_flNextThrowTime = gpGlobals->time;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CMutant::Precache()
{
	PRECACHE_MODEL("models/z_mutant_alpha.mdl");

	PRECACHE_MODEL("sprites/bigspit.spr");// spit projectile.

	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event

	PRECACHE_SOUND("mutant/mutant_eat.wav");
	PRECACHE_SOUND("mutant/mutant_jumpattack.wav");
	PRECACHE_SOUND("mutant/mutant_melee1.wav");
	PRECACHE_SOUND("mutant/mutant_melee2.wav");

	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);

	PRECACHE_SOUND("mutant/mutant_run.wav");

	PRECACHE_SOUND("bullchicken/bc_acid1.wav");

	PRECACHE_SOUND("bullchicken/bc_bite2.wav");
	PRECACHE_SOUND("bullchicken/bc_bite3.wav");

	PRECACHE_SOUND("bullchicken/bc_spithit1.wav");
	PRECACHE_SOUND("bullchicken/bc_spithit2.wav");
}

//=========================================================
// GetSchedule
//=========================================================
Schedule_t *CMutant::GetSchedule( void )
{
	switch( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		{
			// dead enemy
			if( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster::GetSchedule();
			}

			if( HasConditions( bits_COND_NEW_ENEMY ) )
			{
				return GetScheduleOfType( SCHED_WAKE_ANGRY );
			}

			if( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
			}

			if( HasConditions( bits_COND_CAN_MELEE_ATTACK1 ) )
			{
				return GetScheduleOfType( SCHED_MELEE_ATTACK1 );
			}

			if( HasConditions( bits_COND_CAN_MELEE_ATTACK2 ) )
			{
				return GetScheduleOfType( SCHED_MELEE_ATTACK2 );
			}

			return GetScheduleOfType( SCHED_CHASE_ENEMY );
			break;
		}
	default:
			break;
	}

	return CBaseMonster::GetSchedule();
}

// primary range attack
Task_t tlMutantRangeAttack1[] =
{
	{ TASK_STOP_MOVING, 0 },
	{ TASK_FACE_IDEAL, (float)0 },
	{ TASK_RANGE_ATTACK1, (float)0 },
	{ TASK_SET_ACTIVITY, (float)ACT_IDLE },
};

Schedule_t slMutantRangeAttack1[] =
{
	{
		tlMutantRangeAttack1,
		ARRAYSIZE( tlMutantRangeAttack1 ),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_NO_AMMO_LOADED,
		0,
		"Mutant Range Attack1"
	},
};

// Chase enemy schedule
Task_t tlMutantChaseEnemy1[] =
{
	{ TASK_SET_FAIL_SCHEDULE, (float)SCHED_RANGE_ATTACK1 },// !!!OEM - this will stop nasty squid oscillation.
	{ TASK_GET_PATH_TO_ENEMY, (float)0 },
	{ TASK_RUN_PATH, (float)0 },
	{ TASK_WAIT_FOR_MOVEMENT, (float)0 },
};

Schedule_t slMutantChaseEnemy[] =
{
	{
		tlMutantChaseEnemy1,
		ARRAYSIZE( tlMutantChaseEnemy1 ),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_SMELL_FOOD |
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK2 |
		bits_COND_TASK_FAILED,
		0,
		"Mutant Chase Enemy"
	},
};

// victory dance (eating body)
Task_t tlMutantVictoryDance[] =
{
	{ TASK_STOP_MOVING, (float)0 },
	{ TASK_WAIT, (float)0.1 },
	{ TASK_MUTANT_GET_PATH_TO_ENEMY_CORPSE,	(float)0 },
	{ TASK_WALK_PATH, (float)0 },
	{ TASK_WAIT_FOR_MOVEMENT, (float)0 },
	{ TASK_FACE_ENEMY, (float)0 },
	{ TASK_PLAY_SEQUENCE, (float)ACT_VICTORY_DANCE },
	{ TASK_PLAY_SEQUENCE, (float)ACT_VICTORY_DANCE },
	{ TASK_PLAY_SEQUENCE, (float)ACT_VICTORY_DANCE }
};

Schedule_t slMutantVictoryDance[] =
{
	{
		tlMutantVictoryDance,
		ARRAYSIZE( tlMutantVictoryDance ),
		bits_COND_NEW_ENEMY |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE,
		0,
		"MutantVictoryDance"
	},
};

DEFINE_CUSTOM_SCHEDULES( CMutant )
{
	slMutantRangeAttack1,
	slMutantChaseEnemy,
	slMutantVictoryDance,
};

IMPLEMENT_CUSTOM_SCHEDULES( CMutant, CBaseMonster )

Schedule_t* CMutant::GetScheduleOfType(int Type)
{
	switch ( Type )
	{
	case SCHED_RANGE_ATTACK1:
		return &slMutantRangeAttack1[0];
		break;
	case SCHED_CHASE_ENEMY:
		return &slMutantChaseEnemy[0];
		break;
	case SCHED_VICTORY_DANCE:
		return &slMutantVictoryDance[0];
		break;
	default:
		break;
	}
	return CBaseMonster::GetScheduleOfType(Type);
}

void CMutant::RunTask(Task_t *pTask)
{
	// HACK to stop Mutant from playing attack sound twice
	if (pTask->iTask == TASK_MELEE_ATTACK1)
	{
		if (!m_playedAttackSound)
		{
			const char* sample = NULL;
			if (m_meleeAttack2)
			{
				sample = "mutant/mutant_melee2.wav";
			}
			else
			{
				sample = "mutant/mutant_melee1.wav";
			}
			EMIT_SOUND(ENT(pev), CHAN_BODY, sample, 1, ATTN_NORM);
			m_playedAttackSound = true;
		}
	}
	else
	{
		m_playedAttackSound = false;
	}
	CBaseMonster::RunTask(pTask);
}

//=========================================================
// Start task - selects the correct activity and performs
// any necessary calculations to start the next task on the
// schedule.
//=========================================================
void CMutant::StartTask(Task_t *pTask)
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch (pTask->iTask)
	{
	case TASK_MUTANT_GET_PATH_TO_ENEMY_CORPSE:
		{
			UTIL_MakeVectors( pev->angles );
			if( BuildRoute( m_vecEnemyLKP - gpGlobals->v_forward * 40, bits_MF_TO_LOCATION, NULL ) )
			{
				TaskComplete();
			}
			else
			{
				ALERT( at_aiconsole, "MutantGetPathToEnemyCorpse failed!!\n" );
				TaskFail();
			}
		}
		break;
	default:
		CBaseMonster::StartTask(pTask);
		break;

	}
}

//=========================================================
// DEAD MUTANT PROP
//=========================================================
class CDeadMutant : public CBaseMonster
{
public:
	void Spawn(void);
	int	Classify(void) { return	CLASS_ALIEN_MONSTER; }
	void KeyValue( KeyValueData *pkvd );
	int m_iPose;
	static const char *m_szPoses[3];
};

const char *CDeadMutant::m_szPoses[] = { "dead_on_stomach1", "dead_on_back", "dead_on_side" };

void CDeadMutant::KeyValue( KeyValueData *pkvd )
{
	if( FStrEq( pkvd->szKeyName, "pose" ) )
	{
		m_iPose = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue( pkvd );
}

LINK_ENTITY_TO_CLASS(monster_mutant_dead, CDeadMutant)

//=========================================================
// ********** DeadMutant SPAWN **********
//=========================================================
void CDeadMutant::Spawn(void)
{
	PRECACHE_MODEL("models/z_mutant_alpha.mdl");
	SET_MODEL(ENT(pev), "models/z_mutant_alpha.mdl");

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	m_bloodColor = BLOOD_COLOR_RED;

	pev->sequence = LookupSequence( m_szPoses[m_iPose] );
	if( pev->sequence == -1 )
	{
		ALERT( at_console, "Dead mutant with bad pose\n" );
	}

	// Corpses have less health
	pev->health = 8;

	MonsterInitDead();
}