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
// cg_predict.c -- this file generates cg.predictedPlayerState by either
// interpolating between snapshots from the server or locally predicting
// ahead the client's movement.
// It also handles local physics interaction, like fragments bouncing off walls
#include "cg_local.h"
static	pmove_t		cg_pmove;
static	int			cg_numSolidEntities;
static	centity_t	*cg_solidEntities[MAX_ENTITIES_IN_SNAPSHOT];
static	int			cg_numTriggerEntities;
static	centity_t	*cg_triggerEntities[MAX_ENTITIES_IN_SNAPSHOT];
//When a new cg.snap has been set, this function builds a sublist
//of the entities that are actually solid, to make for more
//efficient collision detection
void CG_BuildSolidList(void){
	int				i=0;
	centity_t		*cent;
	snapshot_t		*snap;
	entityState_t	*ent;
	cg_numSolidEntities = 0;
	cg_numTriggerEntities = 0;
	if(cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport){snap = cg.nextSnap;}
	else{snap = cg.snap;}
	for(;i<snap->numEntities;i++){
		cent = &cg_entities[snap->entities[i].number];
		ent = &cent->currentState;
		if(ent->eType == ET_ITEM || ent->eType == ET_PUSH_TRIGGER || ent->eType == ET_TELEPORT_TRIGGER){
			cg_triggerEntities[cg_numTriggerEntities] = cent;
			cg_numTriggerEntities++;
			continue;
		}
		if ( cent->nextState.solid ) {
			cg_solidEntities[cg_numSolidEntities] = cent;
			cg_numSolidEntities++;
			continue;
		}
	}
}
static void CG_ClipMoveToEntities(const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask, trace_t *tr){
	int			i=0, x, zd, zu;
	trace_t		trace;
	entityState_t	*ent;
	clipHandle_t 	cmodel;
	vec3_t		bmins, bmaxs;
	vec3_t		origin, angles;
	centity_t	*cent;
	for(;i<cg_numSolidEntities;i++){
		cent = cg_solidEntities[i];
		ent = &cent->currentState;
		if(ent->number == skipNumber){continue;}
		if(ent->solid == SOLID_BMODEL){
			// special value for bmodel
			cmodel = trap_CM_InlineModel(ent->modelindex);
			VectorCopy(cent->lerpAngles, angles);
			BG_EvaluateTrajectory(&cent->currentState, &cent->currentState.pos, cg.physicsTime, origin);
		}
		else{
			// encoded bbox
			x = (ent->solid & 255);
			zd = ((ent->solid>>8) & 255);
			zu = ((ent->solid>>16) & 255) -32;
			bmins[0] = bmins[1] = -x;
			bmaxs[0] = bmaxs[1] = x;
			bmins[2] = -zd;
			bmaxs[2] = zu;
			cmodel = trap_CM_TempBoxModel(bmins, bmaxs);
			VectorCopy(vec3_origin, angles);
			VectorCopy(cent->lerpOrigin, origin);
		}
		trap_CM_TransformedBoxTrace(&trace, start, end, mins, maxs, cmodel,  mask, origin, angles);
		if(trace.allsolid || trace.fraction < tr->fraction){
			trace.entityNum = ent->number;
			*tr = trace;
		}
		else if(trace.startsolid){tr->startsolid = qtrue;}
		if(tr->allsolid){return;}
	}
}
void CG_Trace(trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask){
	trace_t	t;
	trap_CM_BoxTrace(&t, start, end, mins, maxs, 0, mask);
	t.entityNum = t.fraction != 1.f ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	// check all other solid models
	CG_ClipMoveToEntities(start, mins, maxs, end, skipNumber, mask, &t);
	*result = t;
}

//JUHOX
void CG_SmoothTrace(trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask){
	cg.physicsTime = cg.time;
	CG_Trace(result, start, mins, maxs, end, skipNumber, mask);
	cg.physicsTime = cg.time;
}
int CG_PointContents(const vec3_t point, int passEntityNum){
	int				i=0;
	entityState_t	*ent;
	centity_t		*cent;
	clipHandle_t	cmodel;
	int				contents = trap_CM_PointContents(point, 0);
	for(;i<cg_numSolidEntities;i++){
		cent = cg_solidEntities[i];
		ent = &cent->currentState;
		if(ent->number == passEntityNum){continue;}
		// special value for bmodel
		if(ent->solid != SOLID_BMODEL){continue;}
		cmodel = trap_CM_InlineModel(ent->modelindex);
		if(!cmodel){continue;}
		contents |= trap_CM_TransformedPointContents(point, cmodel, cent->lerpOrigin, cent->lerpAngles);
	}
	return contents;
}
//Generates cg.predictedPlayerState by interpolating between
//cg.snap->player_state and cg.nextFrame->player_state
static void CG_InterpolatePlayerState(qboolean grabAngles){
	float			f;
	int				i;
	playerState_t	*out;
	snapshot_t		*prev, *next;
	out = &cg.predictedPlayerState;
	prev = cg.snap;
	next = cg.nextSnap;
	*out = cg.snap->ps;
	f = (float)(cg.time - prev->serverTime) / (next->serverTime - prev->serverTime);
	i = next->ps.bobCycle;
	// if we are still allowing local input, short circuit the view angles
	if(grabAngles){
		usercmd_t	cmd;
		int	cmdNum;
		cmdNum = trap_GetCurrentCmdNumber();
		trap_GetUserCmd(cmdNum,&cmd);
		PM_UpdateViewAngles(out,&cmd);
	}
	if(cg.nextFrameTeleport || !next || next->serverTime <= prev->serverTime){return;}
	if(i<prev->ps.bobCycle){i += 256;}
	out->bobCycle = prev->ps.bobCycle + f * (i-prev->ps.bobCycle);
	for(i=0;i<3;i++){
		out->origin[i] = prev->ps.origin[i] + f * (next->ps.origin[i] - prev->ps.origin[i]);
		if(!grabAngles){out->viewangles[i] = LerpAngle(prev->ps.viewangles[i],next->ps.viewangles[i],f);}
		out->velocity[i] = prev->ps.velocity[i] + f * (next->ps.velocity[i] - prev->ps.velocity[i]);
	}
}
static void CG_TouchItem(centity_t *cent){return;}
//Predict push triggers and items
static void CG_TouchTriggerPrediction(void){
	int				i=0;
	trace_t			trace;
	entityState_t	*ent;
	clipHandle_t 	cmodel;
	centity_t		*cent;
	qboolean		spectator;
	// dead clients don't activate triggers
	if(cg.predictedPlayerState.powerLevel[plFatigue] <= 0){return;}
	// JUHOX: don't touch triggers in lens flare editor
#if MAPLENSFLARES
	if(cgs.editMode == EM_mlf){return;}
#endif
	spectator = (cg.predictedPlayerState.pm_type == PM_SPECTATOR);
	if(cg.predictedPlayerState.pm_type != PM_NORMAL && !spectator){return;}
	for(;i<cg_numTriggerEntities;i++){
		cent = cg_triggerEntities[i];
		ent = &cent->currentState;
		if(ent->eType == ET_ITEM && !spectator){
			CG_TouchItem(cent);
			continue;
		}
		if(ent->solid != SOLID_BMODEL){continue;}
		cmodel = trap_CM_InlineModel(ent->modelindex);
		if (!cmodel){continue;}
		trap_CM_BoxTrace(&trace, cg.predictedPlayerState.origin, cg.predictedPlayerState.origin, cg_pmove.mins, cg_pmove.maxs, cmodel, -1);
		if(!trace.startsolid){continue;}
		if(ent->eType == ET_TELEPORT_TRIGGER){cg.hyperspace = qtrue;}
		else if (ent->eType == ET_PUSH_TRIGGER){BG_TouchJumpPad(&cg.predictedPlayerState, ent);}
	}
	// if we didn't touch a jump pad this pmove frame
	if(cg.predictedPlayerState.jumppad_frame != cg.predictedPlayerState.pmove_framecount){
		cg.predictedPlayerState.jumppad_frame = 0;
		cg.predictedPlayerState.jumppad_ent = 0;
	}
}
/*
Generates cg.predictedPlayerState for the current cg.time
cg.predictedPlayerState is guaranteed to be valid after exiting.
For demo playback, this will be an interpolation between two valid
playerState_t.
For normal gameplay, it will be the result of predicted usercmd_t on
top of the most recent playerState_t received from the server.
Each new snapshot will usually have one or more new usercmd over the last,
but we simulate all unacknowledged commands each time, not just the new ones.
This means that on an internet connection, quite a few pmoves may be issued
each frame.
OPTIMIZE: don't re-simulate unless the newly arrived snapshot playerState_t
differs from the predicted one.  Would require saving all intermediate
playerState_t during prediction.
We detect prediction errors and allow them to be decayed off over several frames
to ease the jerk.
*/
void CG_PredictPlayerState(void){
	int				cmdNum, current;
	playerState_t	oldPlayerState;
	qboolean		moved;
	usercmd_t		oldestCmd;
	usercmd_t		latestCmd;
	cg.hyperspace = qfalse;
	if(!cg.validPPS){
		cg.validPPS = qtrue;
		cg.predictedPlayerState = cg.snap->ps;
	}
	if(cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) || cg.snap->ps.lockedTarget > 0){
		CG_InterpolatePlayerState(qfalse);
		return;
	}
	cg_pmove.ps = &cg.predictedPlayerState;
	cg_pmove.trace = CG_Trace;
	cg_pmove.pointcontents = CG_PointContents;
	cg_pmove.tracemask = MASK_PLAYERSOLID;
	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR){
		cg_pmove.tracemask &= ~CONTENTS_BODY;
	}
	cg_pmove.noFootsteps = (cgs.dmflags & DF_NO_FOOTSTEPS) > 0;
	oldPlayerState = cg.predictedPlayerState;
	current = trap_GetCurrentCmdNumber();
	// if we don't have the commands right after the snapshot, we
	// can't accurately predict a current position, so just freeze at
	// the last good position we had
	cmdNum = current - CMD_BACKUP + 1;
	trap_GetUserCmd(cmdNum, &oldestCmd);
	if(oldestCmd.serverTime > cg.snap->ps.commandTime && oldestCmd.serverTime < cg.time){
		if(cg_showmiss.integer){CG_Printf ("exceeded PACKET_BACKUP on commands\n");}
		return;
	}
	// get the latest command so we can know which commands are from previous map_restarts
	trap_GetUserCmd(current, &latestCmd);
	// get the most recent information we have, even if
	// the server time is beyond our current cg.time,
	// because predicted player positions are going to 
	// be ahead of everything else anyway
	if(cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport){
		cg.predictedPlayerState = cg.nextSnap->ps;
		cg.physicsTime = cg.nextSnap->serverTime;
	}
	else{
		cg.predictedPlayerState = cg.snap->ps;
		cg.physicsTime = cg.snap->serverTime;
	}
	if(pmove_msec.integer < 8){trap_Cvar_Set("pmove_msec", "8");}
	else if(pmove_msec.integer > 33){trap_Cvar_Set("pmove_msec", "33");}
	//Eagle: What is that comment for?
	cg_pmove.pmove_fixed = pmove_fixed.integer;// | cg_pmove_fixed.integer;
	cg_pmove.pmove_msec = pmove_msec.integer;
	//run cmds
	moved = qfalse;
	cmdNum = current - CMD_BACKUP + 1;
	for(;cmdNum <= current; cmdNum++){
		//get the command
		trap_GetUserCmd(cmdNum, &cg_pmove.cmd);
		if(cg_pmove.pmove_fixed){PM_UpdateViewAngles(cg_pmove.ps, &cg_pmove.cmd);}
		// don't do anything if the time is before the snapshot player time
		if(cg_pmove.cmd.serverTime <= cg.predictedPlayerState.commandTime){continue;}
		// don't do anything if the command was from a previous map_restart
		if (cg_pmove.cmd.serverTime > latestCmd.serverTime){continue;}
		// check for a prediction error from last frame
		// on a lan, this will often be the exact value
		// from the snapshot, but on a wan we will have
		// to predict several commands to get to the point
		// we want to compare
		if(cg.predictedPlayerState.commandTime == oldPlayerState.commandTime){
			vec3_t	delta;
			float	len;
			if(cg.thisFrameTeleport){
				// a teleport will not cause an error decay
				VectorClear(cg.predictedError);
				if(cg_showmiss.integer){CG_Printf("PredictionTeleport\n");}
				cg.thisFrameTeleport = qfalse;
			}
			else{
				vec3_t	adjusted;
				CG_AdjustPositionForMover(cg.predictedPlayerState.origin, cg.predictedPlayerState.groundEntityNum, cg.physicsTime, cg.oldTime, adjusted);
				if(cg_showmiss.integer && !VectorCompare(oldPlayerState.origin, adjusted)){
						CG_Printf("prediction error\n");
				}
				VectorSubtract(oldPlayerState.origin, adjusted, delta);
				len = VectorLength(delta);
				if(len > .1f){
					if(cg_showmiss.integer){CG_Printf("Prediction miss: %f\n", len);}
					if(cg_errorDecay.integer){
						int		t = cg.time - cg.predictedErrorTime;
						float	f = (cg_errorDecay.value - t) / cg_errorDecay.value;
						if(f < 0){f = 0;}
						if(f > 0 && cg_showmiss.integer){CG_Printf("Double prediction decay: %f\n", f);}
						VectorScale(cg.predictedError, f, cg.predictedError);
					}
					else{VectorClear(cg.predictedError);}
					VectorAdd(delta, cg.predictedError, cg.predictedError);
					cg.predictedErrorTime = cg.oldTime;
				}
			}
		}
		// don't predict gauntlet firing, which is only supposed to happen
		// when it actually inflicts damage
		cg_pmove.gauntletHit = qfalse;
		if(cg_pmove.pmove_fixed){
			cg_pmove.cmd.serverTime = ((cg_pmove.cmd.serverTime + pmove_msec.integer - 1) / pmove_msec.integer) * pmove_msec.integer;
		}
		Pmove(&cg_pmove);
		moved = qtrue;
		// add push trigger movement effects
		CG_TouchTriggerPrediction();
		// check for predictable events that changed from previous predictions
		//CG_CheckChangedPredictableEvents(&cg.predictedPlayerState);
	}
	if(cg_showmiss.integer > 1){CG_Printf("[%i : %i] ", cg_pmove.cmd.serverTime, cg.time);}
	if(!moved){
		if(cg_showmiss.integer){CG_Printf("not moved\n");}
		return;
	}
	// adjust for the movement of the groundentity
	CG_AdjustPositionForMover(cg.predictedPlayerState.origin,cg.predictedPlayerState.groundEntityNum,cg.physicsTime, cg.time, cg.predictedPlayerState.origin);
	if(cg_showmiss.integer){
		if(cg.predictedPlayerState.eventSequence > oldPlayerState.eventSequence + MAX_PS_EVENTS){
			CG_Printf("WARNING: dropped event\n");
		}
	}
	// fire events and other transition triggered things
	CG_TransitionPlayerState(&cg.predictedPlayerState,&oldPlayerState);
	if(cg_showmiss.integer){
		if(cg.eventSequence > cg.predictedPlayerState.eventSequence){
			CG_Printf("WARNING: double event\n");
			cg.eventSequence = cg.predictedPlayerState.eventSequence;
		}
	}
	if(cg.snap->ps.lockedTarget > 0){
		CG_InterpolatePlayerState(qfalse);
		return;
	}
}
