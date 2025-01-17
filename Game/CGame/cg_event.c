/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// cg_event.c -- handle entity events at snapshot or playerstate transitions
#include "cg_local.h"
#include "../../Shared/q_shared.h"

/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/
#define	DEBUGNAME(x) if(cg_debugEvents.integer) CG_Printf(x"\n");
void CG_EntityEvent(centity_t *cent, vec3_t position){
	entityState_t	*es;
	clientInfo_t	*ci;
	vec3_t			dir;
	int				r, size, event, clientNum;
	const char		*s;

	r = random() *10;
	es = &cent->currentState;
	event = es->event & ~EV_EVENT_BITS;
	if(cg_debugEvents.integer)
		CG_Printf("Entity:%3i  Event:%3i", es->number, event);
	if(!event){
		DEBUGNAME("ZEROEVENT");
		return;
	}
	clientNum = es->clientNum;
	if(clientNum < 0 || clientNum >= MAX_CLIENTS) clientNum = 0;
	ci = &cgs.clientinfo[clientNum];
	switch(event){
	//
	// movement generated events
	//
	case EV_NULL:
		break;
	case EV_CRASH:
		DEBUGNAME("EV_CRASH");
		trap_S_StartSound(NULL, es->number, CHAN_AUTO, CG_CustomSound(es->number, "landHeavy"));
		PSys_SpawnCachedSystem("AuraDebris", cent->lerpOrigin, NULL, cent, NULL, qtrue, qfalse);
		break;
	case EV_FOOTSTEP:
		DEBUGNAME("EV_FOOTSTEP");
		if(cg_footsteps.integer) trap_S_StartSound(NULL, es->number, CHAN_AUTO, CG_CustomSound(es->number, "footsteps"));
		break;
	case EV_FOOTSTEP_METAL:
		DEBUGNAME("EV_FOOTSTEP_METAL");
		if(cg_footsteps.integer) trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_METAL][rand()&3]);
		break;
	case EV_FOOTSPLASH:
		DEBUGNAME("EV_FOOTSPLASH");
		if(cg_footsteps.integer) trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_SPLASH][rand()&3]);
		break;
	case EV_FOOTWADE:
		DEBUGNAME("EV_FOOTWADE");
		if(cg_footsteps.integer) trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_SPLASH][rand()&3]);
		break;
	case EV_SWIM:
		DEBUGNAME("EV_SWIM");
		if(cg_footsteps.integer) trap_S_StartSound(NULL, es->number, CHAN_BODY, cgs.media.footsteps[FOOTSTEP_SPLASH][rand()&3]);
		break;
	case EV_LAND_LIGHT:
		DEBUGNAME("EV_LAND_LIGHT");
		trap_S_StartSound(NULL, es->number, CHAN_AUTO, CG_CustomSound(es->number, "landLight"));
		CG_PlayerDirtPush(cent,2,qtrue);
		if(clientNum == cg.predictedPlayerState.clientNum){
			cg.landChange = -8;
			cg.landTime = cg.time;
		}
		break;
	case EV_LAND_MEDIUM:
		DEBUGNAME("EV_LAND_MEDIUM");
		trap_S_StartSound(NULL, es->number, CHAN_AUTO, CG_CustomSound(es->number, "landMedium"));
		CG_PlayerDirtPush(cent,4,qtrue);
		if(clientNum == cg.predictedPlayerState.clientNum){
			cg.landChange = -16;
			cg.landTime = cg.time;
		}
		break;
	case EV_LAND_HEAVY:
		DEBUGNAME("EV_LAND_HEAVY");
		trap_S_StartSound(NULL, es->number, CHAN_AUTO, CG_CustomSound(es->number, "landHeavy"));
		CG_PlayerDirtPush(cent,8,qtrue);
		if(clientNum == cg.predictedPlayerState.clientNum){
			cg.landChange = -24;
			cg.landTime = cg.time;
		}
		break;
	case EV_STEP_4:
	case EV_STEP_8:
	case EV_STEP_12:
	// smooth out step up transitions
	case EV_STEP_16:
		DEBUGNAME("EV_STEP");
	{
		float	oldStep;
		int		delta, step;

		if(clientNum != cg.predictedPlayerState.clientNum) break;
		// if we are interpolating, we don't need to smooth steps
		if(cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ||
			cg_nopredict.integer || cg_synchronousClients.integer) break;
		// check for stepping up before a previous step is completed
		delta = cg.time -cg.stepTime;
		if(delta < STEP_TIME)
			 oldStep = cg.stepChange *(STEP_TIME -delta) / STEP_TIME;
		else oldStep = 0;
		// add this amount
		step = (event -EV_STEP_4 +1) *4;
		cg.stepChange = oldStep +step;
		if(cg.stepChange > MAX_STEP_CHANGE)
			cg.stepChange = MAX_STEP_CHANGE;
		cg.stepTime = cg.time;
		break;
	}
	case EV_JUMP_PAD:
		DEBUGNAME("EV_JUMP_PAD");
		break;
	case EV_JUMP:
		DEBUGNAME("EV_JUMP");
		trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "jump"));
		CG_PlayerDirtPush(cent,10,qtrue);
		break;
	case EV_JUMP_HIGH:
		DEBUGNAME("EV_JUMP_HIGH");
		trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "jumpHigh"));
		CG_PlayerDirtPush(cent,20,qtrue);
		break;
	case EV_LOCKON_START:
		DEBUGNAME("EV_LOCKON_START");
		if(!cg.snap->ps.lockedTarget){
			ci->lockStartTimer = cg.time +800;
			trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.lockonStart);
			trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "taunt"));
		}
		break;
	case EV_LOCKON_RESTART:
		DEBUGNAME("EV_LOCKON_RESTART");
		ci->lockStartTimer = cg.time +800;
		trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.lockonStart);
		break;
	case EV_LOCKON_END:
		DEBUGNAME("EV_LOCKON_END");
		break;
	case EV_STUNNED:
		DEBUGNAME("EV_STUNNED");
		trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "pain"));
		break;
	case EV_BLOCK:
		DEBUGNAME("EV_BLOCK");
		trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "pain"));
		break;
	case EV_PUSH:
		DEBUGNAME("EV_PUSH");
		trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "pain"));
		break;
	case EV_SWAT:
		DEBUGNAME("EV_SWAT");
		trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, "pain"));
		break;
	case EV_WATER_TOUCH:
		DEBUGNAME("EV_WATER_TOUCH");
		trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.watrInSound);
		break;
	case EV_WATER_LEAVE:
		DEBUGNAME("EV_WATER_LEAVE");
		trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound);
		break;
	case EV_WATER_UNDER:
		DEBUGNAME("EV_WATER_UNDER");
		trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound);
		break;
	case EV_WATER_CLEAR:
		DEBUGNAME("EV_WATER_CLEAR");
		trap_S_StartSound(NULL, es->number, CHAN_AUTO, CG_CustomSound(es->number, "gasp"));
		break;
	case EV_WATER_SPLASH:
		DEBUGNAME("EV_WATER_SPLASH");
		CG_WaterSplash(es->origin,1);
		break;
	//
	// weapon events
	//
	case EV_CHANGE_WEAPON:
		DEBUGNAME("EV_CHANGE_WEAPON");
		break;
	case EV_FIRE_WEAPON:
		DEBUGNAME("EV_FIRE_WEAPON");
		CG_FireWeapon(cent, qfalse);
		break;
	case EV_ALTFIRE_WEAPON:
		DEBUGNAME("EV_ALTFIRE_WEAPON");
		CG_FireWeapon(cent, qtrue);
		break;
	case EV_DETONATE_WEAPON:
		DEBUGNAME("EV_DETONATE_WEAPON");
		break;
	case EV_ZANZOKEN_START:
	case EV_ZANZOKEN_END:
		DEBUGNAME("EV_ZANZOKEN");
		CG_SpawnEffect( position );
		CG_SpawnLightSpeedGhost( cent );
		break;
	case EV_PLAYER_TELEPORT_IN:
		DEBUGNAME("EV_PLAYER_TELEPORT_IN");
		trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.teleInSound);
		CG_SpawnEffect(position);
		break;
	case EV_PLAYER_TELEPORT_OUT:
		DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
		trap_S_StartSound(NULL, es->number, CHAN_AUTO, cgs.media.teleOutSound);
		CG_SpawnEffect(position);
		break;
	case EV_MISSILE_HIT:
		DEBUGNAME("EV_MISSILE_HIT");
		ByteToDir(es->eventParm, dir);
		CG_UserMissileHitPlayer(es->weapon, es->clientNum, es->powerups, es->number, position, dir, es->otherEntityNum);
		break;
	case EV_MISSILE_MISS:
		DEBUGNAME("EV_MISSILE_MISS");
		ByteToDir(es->eventParm, dir);
		CG_UserMissileHitWall(es->weapon, es->clientNum, es->powerups, es->number, position, dir, qfalse);
		break;
	case EV_MISSILE_MISS_METAL:
		DEBUGNAME("EV_MISSILE_MISS_METAL");
		ByteToDir(es->eventParm, dir);
		CG_UserMissileHitWall(es->weapon, es->clientNum, es->powerups, es->number, position, dir, qfalse);
		break;
	case EV_MISSILE_MISS_AIR:
		DEBUGNAME("EV_MISSILE_MISS_AIR");
		ByteToDir(es->eventParm, dir);
		CG_UserMissileHitWall(es->weapon, es->clientNum, es->powerups, es->number, position, dir, qtrue);	
	case EV_BEAM_FADE:
		DEBUGNAME("EV_BEAM_FADE");
		break;
	case EV_GENERAL_SOUND:
		DEBUGNAME("EV_GENERAL_SOUND");
		if(cgs.gameSounds[es->eventParm])
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
		else{
			s = CG_ConfigString(CS_SOUNDS +es->eventParm);
			trap_S_StartSound(NULL, es->number, CHAN_VOICE, CG_CustomSound(es->number, s));
		}
		break;
	case EV_GLOBAL_SOUND:
		DEBUGNAME("EV_GLOBAL_SOUND");
		if(cgs.gameSounds[es->eventParm])
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.gameSounds[ es->eventParm ] );
		else{
			s = CG_ConfigString(CS_SOUNDS +es->eventParm);
			trap_S_StartSound(NULL, cg.snap->ps.clientNum, CHAN_AUTO, CG_CustomSound(es->number, s));
		}
		break;
	case EV_EARTHQUAKE:
		DEBUGNAME("EV_EARTHQUAKE");
		CG_AddEarthquake(es->origin, es->angles2[1], es->angles[0],
							es->angles[1], es->angles[2], es->angles2[0]);
		if(!es->time){
			cg.earthquakeSoundCounter = (cg.earthquakeEndTime -cg.earthquakeStartedTime) / 200;
			cg.lastEarhquakeSoundStartedTime = cg.time -1000;
		}
		break;
	case EV_AIRBRAKE:
		DEBUGNAME("EV_AIRBRAKE");
		{
			if(r > 2.5f)	trap_S_StartSound(cent->lerpOrigin,es->number,CHAN_BODY,cgs.media.airBrake1);
			else			trap_S_StartSound(cent->lerpOrigin,es->number,CHAN_BODY,cgs.media.airBrake2);
			CG_AddEarthquake(cent->lerpOrigin, 1000, 1, 0, 1, 500);
			CG_PowerMeleeEffect(cent->lerpOrigin, 3);
			break;
		}
	case EV_PAIN_LIGHT:
		DEBUGNAME("EV_PAIN_LIGHT");
		trap_S_StartSound(NULL, es->number,CHAN_VOICE,CG_CustomSound(es->number,"painLight"));
		break;
	case EV_PAIN_MEDIUM:
		DEBUGNAME("EV_PAIN_MEDIUM");
		trap_S_StartSound(NULL, es->number,CHAN_VOICE,CG_CustomSound(es->number,"painMedium"));
		break;
	case EV_PAIN_HEAVY:
		DEBUGNAME("EV_PAIN_HEAVY");
		trap_S_StartSound(NULL, es->number,CHAN_VOICE,CG_CustomSound(es->number,"painHeavy"));
		break;
	case EV_DEATH:
		DEBUGNAME("EV_DEATH");
		trap_S_StartSound(NULL, es->number,CHAN_VOICE,CG_CustomSound(es->number,"death"));
		break;
	case EV_UNCONCIOUS:
		DEBUGNAME("EV_UNCONCIOUS");
		trap_S_StartSound(NULL, es->number,CHAN_VOICE,CG_CustomSound(es->number,"death"));
		break;
	case EV_STOPLOOPINGSOUND:
		DEBUGNAME("EV_STOPLOOPINGSOUND");
		trap_S_StopLoopingSound(es->number);
		es->loopSound = 0;
		break;
	case EV_DEBUG_LINE:
		DEBUGNAME("EV_DEBUG_LINE");
		CG_Beam(cent);
		break;
	case EV_BALLFLIP:
		DEBUGNAME("EV_BALLFLIP");
		trap_S_AddRealLoopingSound(es->number,cent->lerpOrigin,vec3_origin,CG_CustomSound(es->number,"ballFlip"));
		break;
	case EV_MELEE_SPEED:
		DEBUGNAME("EV_MELEE_SPEED");
		trap_S_AddRealLoopingSound(es->number,cent->lerpOrigin,vec3_origin,cgs.media.speedMeleeSound);
		CG_SpeedMeleeEffect(cent->lerpOrigin,cent->currentState.tier);
		break;
	case EV_MELEE_MISS:
		DEBUGNAME("EV_MELEE_MISS");
		trap_S_AddRealLoopingSound(cent->currentState.number,cent->lerpOrigin,vec3_origin,cgs.media.speedMissSound);
		break;
	case EV_MELEE_KNOCKBACK:
		DEBUGNAME("EV_MELEE_KNOCKBACK");
		trap_S_StartSound(cent->lerpOrigin,es->number,CHAN_BODY,cgs.media.powerMeleeSound);
		CG_AddEarthquake(cent->lerpOrigin, 1000, 1, 0, 1, 500);
		CG_PowerMeleeEffect(cent->lerpOrigin,cent->currentState.tier);
		break;
	case EV_MELEE_STUN:
		DEBUGNAME("EV_MELEE_STUN");
		break;
	case EV_MELEE_KNOCKOUT:
		DEBUGNAME("EV_MELEE_KNOCKOUT");
		break;
	case EV_MELEE_BREAKER:
		DEBUGNAME("EV_MELEE_BREAKER");
		trap_S_StartSound(cent->lerpOrigin,es->number,CHAN_BODY,cgs.media.powerStunSound1);
		break;
	case EV_TIERUP_FIRST:
		DEBUGNAME("EV_TIERUP_FIRST");
		trap_S_StartSound(cent->lerpOrigin,es->number,CHAN_BODY,ci->tierConfig[ci->tierCurrent].soundTransformFirst);
		break;
	case EV_TIERUP:
		DEBUGNAME("EV_TIERUP");
		trap_S_StartSound(cent->lerpOrigin,es->number,CHAN_BODY,ci->tierConfig[ci->tierCurrent].soundTransformUp);
		break;
	case EV_TIERDOWN:
		DEBUGNAME("EV_TIERDOWN");
		trap_S_StartSound(cent->lerpOrigin,es->number,CHAN_BODY,ci->tierConfig[ci->tierCurrent].soundTransformDown);
		break;
	case EV_SYNCTIER:
		DEBUGNAME("EV_SYNCTIER");
		break;
	case EV_ALTERUP_START:
		DEBUGNAME("EV_ALTERUP_START");
		trap_S_StartSound(cent->lerpOrigin,es->number,CHAN_BODY,ci->auraConfig[ci->tierCurrent]->boostStartSound);
		break;
	case EV_ALTERDOWN_START:
		DEBUGNAME("EV_ALTERDOWN_START");
		break;
	case EV_POWERINGUP_START:
		DEBUGNAME("EV_POWERINGUP_START");
		trap_S_StartSound(cent->lerpOrigin,es->number,CHAN_BODY,ci->tierConfig[ci->tierCurrent].soundPoweringUp);
		break;
	case EV_BOOST_START:
		DEBUGNAME("EV_BOOST_START");
		trap_S_StartSound(cent->lerpOrigin, es->number,CHAN_BODY,ci->auraConfig[ci->tierCurrent]->boostStartSound);
		break;
	case EV_POWER_STRUGGLE_START:
		DEBUGNAME("EV_POWER_STRUGGLE_START");
		CG_AddEarthquake(position, 20000, 1, 0, 1, 100);
		if(es->dashDir[1] > 300)		size = 4;
		else if(es->dashDir[1] > 200)	size = 3;
		else if(es->dashDir[1] > 100)	size = 2;
		else if(es->dashDir[1] > 50)	size = 1;
		else							size = 0;
		CG_PowerStruggleEffect(position,size);
		trap_S_StartSound(position,es->number,CHAN_BODY,cgs.media.airBrake1);
		break;
	case EV_HOVER:
		DEBUGNAME("EV_HOVER");
		trap_S_StartSound(position,es->number,CHAN_BODY,cgs.media.hover);
		break;
	case EV_HOVER_FAST:
		DEBUGNAME("EV_HOVER_FAST");
		trap_S_StartSound(position,es->number,CHAN_BODY,cgs.media.hoverFast);
		break;
	case EV_HOVER_LONG:
		DEBUGNAME("EV_HOVER_LONG");
		trap_S_StartSound(position,es->number,CHAN_BODY,cgs.media.hoverLong);
		break;
	default:
		DEBUGNAME("UNKNOWN");
		CG_Error("Unknown event: %i", event);
		return;
	}
}

/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents(centity_t *cent){
	// check for event-only entities
	if(cent->currentState.eType > ET_EVENTS){
		// already fired
		if(cent->previousEvent) return;	
		// if this is a player event set the entity number of the client entity number
		if(cent->currentState.eFlags & EF_PLAYER_EVENT)
			cent->currentState.number = cent->currentState.otherEntityNum;
		cent->previousEvent = 1;
		cent->currentState.event = cent->currentState.eType -ET_EVENTS;
	}
	else{
		// check for events riding with another entity
		if(cent->currentState.event == cent->previousEvent) return;
		cent->previousEvent = cent->currentState.event;
		if(!(cent->currentState.event & ~EV_EVENT_BITS)) return;
	}
	// calculate the position at exactly the frame time
	BG_EvaluateTrajectory(&cent->currentState, &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin);
	CG_SetEntitySoundPosition(cent);
	CG_EntityEvent(cent, cent->lerpOrigin);
}
