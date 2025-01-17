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
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay
#include "cg_local.h"

int sortedTeamPlayers[TEAM_MAXOVERLAY];
int	numSortedTeamPlayers;

/*
=================
CG_DrawHorGauge

=================
*/
void CG_DrawHorGauge(float x, float y, float w, float h, vec4_t color_bar, vec4_t color_empty, int value, int maxvalue, qboolean reversed){
	float pct, bar_w;

	pct = (float)value / (float)maxvalue;
	bar_w = w * pct;

	if(color_empty[3]){
		trap_R_SetColor(color_empty);
		if(!reversed)	CG_DrawPic(qfalse, x +bar_w, y, w -bar_w, h, cgs.media.whiteShader);
		else 			CG_DrawPic(qfalse, x -bar_w, y, w +bar_w, h, cgs.media.whiteShader);
	}
	trap_R_SetColor(color_bar);
	if(bar_w > w) bar_w = w;
	if(!bar_w){
		trap_R_SetColor(NULL);
		return;
	}
	if(!reversed)	CG_DrawPic(qfalse, x, y, bar_w, h, cgs.media.whiteShader);
	else			CG_DrawPic(qfalse, x +w -bar_w, y, bar_w, h, cgs.media.whiteShader);
	trap_R_SetColor(NULL);
}

void CG_DrawDiffGauge(float x, float y, float width, float height, vec4_t color, vec4_t empty, int base, int value, int maxValue, int direction){
	float percent;
	int newWidth, newX, difference;
	
	difference = base -value;
	if((difference > 0 && direction <= 0) || (difference < 0 && direction >= 0)) return;
	percent = ((float)value / (float)maxValue);
	newX = x + ((float)(width *percent));
	newWidth = ((float)difference / (float)maxValue) *width;
	CG_DrawHorGauge(newX, y, newWidth, height, color, empty, 1, 1, qfalse);
}

void CG_DrawRightGauge(float x, float y, float width, float height, vec4_t color, vec4_t empty, int value, int maxValue){
	float percent;
	int newWidth, newX, change;
	
	percent = ((float)value / (float)maxValue);
	change = width *percent;
	newX = x + change;
	newWidth = width - change;
	CG_DrawHorGauge(newX, y, newWidth, height, color, empty, 1, 1, qfalse);
}

void CG_DrawReverseGauge(float x, float y, float width, float height, vec4_t color, vec4_t empty, int value, int maxValue){
	float percent;
	int newWidth, newHeight, newX, newY;
	
	percent = (float)value / (float)maxValue;
	newWidth = width *percent;
	newX = (x+width)-newWidth;
	newHeight = height *percent;
	newY = (y+height)-newHeight;
	CG_DrawHorGauge(newX, newY, newWidth, newHeight, color, empty, 1, 1, qfalse);
}

/*
==================
CG_DrawVertGauge

==================
*/
void CG_DrawVertGauge(float x, float y, float w, float h, vec4_t color_bar, vec4_t color_empty, int value, int maxvalue, qboolean reversed){
	float pct, bar_h;

	pct = (float)value / (float)maxvalue;
	bar_h = h *pct;
	if(color_empty[3]){
		trap_R_SetColor(color_empty);
		if(reversed)	CG_DrawPic(qfalse, x, y +bar_h, w, h -bar_h, cgs.media.whiteShader);
		else			CG_DrawPic(qfalse, x, y, w, h -bar_h, cgs.media.whiteShader);
	}
	trap_R_SetColor(color_bar);
	if(bar_h > h) bar_h = h;
	if(!bar_h){
		trap_R_SetColor(NULL);
		return;
	}
	if(reversed)	CG_DrawPic(qfalse, x, y, w, bar_h, cgs.media.whiteShader);
	else			CG_DrawPic(qfalse, x, y +h -bar_h, w, bar_h, cgs.media.whiteShader);
	trap_R_SetColor(NULL);
}

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel(float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles){
	refdef_t		refdef;
	refEntity_t		ent;
	
	CG_AdjustFrom640(&x, &y, &w, &h,qtrue);
	memset(&refdef, 0, sizeof(refdef));
	memset(&ent, 0, sizeof(ent));
	AnglesToAxis(angles, ent.axis);
	VectorCopy(origin, ent.origin);
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW | RF_DEPTHHACK | RF_LIGHTING_ORIGIN;
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear(refdef.viewaxis);
	refdef.fov_x = refdef.fov_y = 30;
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;
	refdef.time = cg.time;
	trap_R_ClearScene();
	trap_R_AddRefEntityToScene(&ent);
	trap_R_RenderScene(&refdef);
}

/*
================
CG_DrawHead

Used for both the status bar and the scoreboard
================
*/
void CG_DrawHead(float x, float y, float w, float h, int clientNum, vec3_t headAngles){
	clipHandle_t	cm;
	clientInfo_t	*ci;
	float			len;
	vec3_t			origin, mins, maxs;
	tierConfig_cg	*tier;
	qhandle_t		icon;
	
	ci = &cgs.clientinfo[clientNum];
	tier = &ci->tierConfig[ci->tierCurrent];
	if(!cg_draw3dIcons.integer){
		icon = tier->icon2D[0];
		CG_DrawPic(qfalse, x, y, w, h,icon);
	}
	else{
		cm = ci->headModel[ci->tierCurrent];
		if(!cm) return;
		trap_R_ModelBounds(cm,mins,maxs,0);
		len = .7f *(maxs[2] -mins[2]);		
		origin[0] = len / (1.f -tier->icon3DZoom);
		origin[1] = .5f *(mins[1] +maxs[1]);
		origin[2] = -.5f *(mins[2] +maxs[2]);
		headAngles[0] = tier->icon3DRotation[0];
		headAngles[1] = tier->icon3DRotation[1];
		headAngles[2] = tier->icon3DRotation[2];
		CG_Draw3DModel(x+tier->icon3DOffset[0], y+tier->icon3DOffset[1], w+tier->icon3DSize[0], h+tier->icon3DSize[1],
						ci->headModel[ci->tierCurrent], ci->headSkin[ci->tierCurrent], origin,headAngles);
	}
}

/*================
CG_DRAWCHAT
================*/
void strrep(char *str, char old, char new) {
    char *pos;
	
    while(1){
        pos = strchr(str, old);
        if(pos == NULL) break;
        *pos = new;
    }
}

void CG_CheckChat(void){
	vec3_t angles;
	int i,
		yStart	= cg.predictedPlayerState.lockedTarget ? 330 : 0,
		yOffset = cg.predictedPlayerState.lockedTarget ? -40 : 40;
	
	if(cg.time < cgs.chatTimer){
		for(i=0;i<3;++i)
			if(cgs.messageClient[i] >= 0){
				CG_DrawPic(qfalse, -15, -15 +yStart +(yOffset*i), 369, 75, cgs.media.chatBackgroundShader);
				CG_DrawHead(7, 7 +yStart +(yOffset*i), 32, 32, cgs.messageClient[i], angles);
				CG_DrawSmallStringCustom(45, 16 +yStart +(yOffset*i), 8, 8, cgs.messages[i], 1.f, 4);
			}
	}
	else
		for(i=0;i<3;++i){
			cgs.messageClient[i] = -1;
			strcpy(cgs.messages[i],"");
		}
}

void CG_DrawChat(char *text){
	char	cleaned[256], name[64], *safe,
			find[]  = "^7";
	int 	safeIndex, clientNum;
	
	safe = text;
	strcpy(cleaned, text);
	strrep(safe, *find, ' ');
	cgs.chatTimer = cg.time +cg_chatTime.integer;
	strcpy(name, COM_Parse(&safe));
	for(safeIndex=0;safeIndex<3;safeIndex++)
		if(!strcmp(cgs.messages[safeIndex], "")) break;
	if(safeIndex >= 2 && cgs.messageClient[2] >= 0){
		safeIndex = 2;
		cgs.messageClient[0] = cgs.messageClient[1];
		cgs.messageClient[1] = cgs.messageClient[2];
		strcpy(cgs.messages[0], cgs.messages[1]);
		strcpy(cgs.messages[1], cgs.messages[2]);
	}
	for(clientNum=0;clientNum<MAX_CLIENTS;clientNum++)
		if(!strcmp(name, cgs.clientinfo[clientNum].name)){
			cgs.messageClient[safeIndex] = clientNum;
			break;
		}
	strcpy(cgs.messages[safeIndex],cleaned);
}

void CG_DrawScreenEffects(void){
	clientInfo_t	*ci;
	tierConfig_cg	*tier;
	playerState_t	*ps;
	qhandle_t		effect;
	
	ci = &cgs.clientinfo[cg.snap->ps.clientNum];
	tier = &ci->tierConfig[ci->tierCurrent];
	ps = &cg.snap->ps;
	effect = tier->screenEffect[ci->damageTextureState-1];
		 if(ps->bitFlags & isBreakingLimit && tier->screenEffectPowering)	 effect = tier->screenEffectPowering;
	else if(ps->bitFlags & isTransforming && tier->screenEffectTransforming) effect = tier->screenEffectTransforming;
	CG_DrawPic(qfalse, 0, 0, 640, 480, effect);
}

/*================
CG_HUD
================*/
void CG_DrawHUD(playerState_t *ps, int clientNum, int x, int y, qboolean flipped){
	const char			*powerLevelString;
	int 				powerLevelOffset;
	long	 			powerLevelDisplay;
	float				multiplier;
	cg_userWeapon_t		*skill;
	int					chargePercent, chargeReady;
	qboolean 			charging = ps->weaponstate == WEAPON_CHARGING || ps->weaponstate == WEAPON_ALTCHARGING ? qtrue : qfalse;
	vec4_t				*chargeColor, angles,
						healthFull =			{1.f, .1f,	  0, 1.f},
						healthEmpty=			{.3f,  .1,	  0, 1.f},
						fatigueFull =			{.4f, .4f,	.5f, 1.f},
						fatigueEmpty =			{.9f, .5f,	  0, 1.f},
						currentFull =			{  0, .6f,	1.f, 1.f},
						currentEmpty =			{.2f, .3f, .35f, 1.f},
						chargeReadyColor =		{.6f, 1.f,	  0, 1.f};

	if(!charging){
		CG_DrawHorGauge(x+60, y+19, 200, 16, healthFull, healthEmpty, ps->powerLevel[plHealth], ps->powerLevel[plMaximum], qfalse);
		CG_DrawHorGauge(x+60, y+43, 200, 16, currentFull, currentEmpty, ps->powerLevel[plCurrent], ps->powerLevel[plMaximum], qfalse);
		CG_DrawRightGauge(x+60, y+43, 200, 16, fatigueFull, fatigueFull, ps->powerLevel[plFatigue], ps->powerLevel[plMaximum]);
		CG_DrawDiffGauge(x+60, y+43, 200, 16, fatigueEmpty, fatigueEmpty, ps->powerLevel[plCurrent], ps->powerLevel[plFatigue], ps->powerLevel[plMaximum], 1);
		CG_DrawPic(qfalse, x, y-22, 288, 72, cgs.media.hudShader);
		CG_DrawPic(qfalse, x, y+2, 288, 72, cgs.media.hudShader);
		CG_DrawHead(x+6, y+14, 50, 50, clientNum, angles);
		if(ps->powerLevel[plCurrent] == ps->powerLevel[plMaximum] && ps->bitFlags & usingAlter)
			CG_DrawPic(qfalse, x+243, y+27, 40, 44, cgs.media.breakLimitShader);
		multiplier = cgs.clientinfo[clientNum].tierConfig[cgs.clientinfo[clientNum].tierCurrent].hudMultiplier;
		if(multiplier <= 0) multiplier = 1.f;
		powerLevelDisplay = (float)ps->powerLevel[plCurrent] *multiplier;
		if(ps->powerLevel[plCurrent] *multiplier == 9001)
			powerLevelString = "Over ^3NINE-THOUSAND!!!";	// WHAT NINE THOUSANDS?!
		else
			powerLevelString = powerLevelDisplay >= 1000000 ? va("%.1f ^3mil", (float)powerLevelDisplay / 1000000.f) : va("%li",powerLevelDisplay);
	}
	else{
		if(ps->weaponstate == WEAPON_CHARGING){
			chargePercent = ps->stats[stChargePercentPrimary];
			chargeReady = ps->currentSkill[WPSTAT_CHRGREADY];
		}
		else{
			chargePercent = ps->stats[stChargePercentSecondary];
			chargeReady = ps->currentSkill[WPSTAT_ALT_CHRGREADY];
		}
		skill = CG_FindUserWeaponGraphics(cg.snap->ps.clientNum,cg.weaponSelect);
		chargeColor = chargePercent >= chargeReady ? &chargeReadyColor : &fatigueEmpty;
		powerLevelDisplay = ps->attackPower;
		powerLevelString = powerLevelDisplay >= 1000000 ? va("%.1f ^3mil", (float)powerLevelDisplay / 1000000.f) : va("%li",powerLevelDisplay);
		CG_DrawHorGauge(x+60, y+43, 200, 16, *chargeColor, currentEmpty, chargePercent, 100, qfalse);
		CG_DrawPic(qfalse, x, y+2, 288, 72, cgs.media.hudShader);
		CG_DrawPic(qfalse, x+6, y+22, 50, 50, skill->weaponIcon);
		CG_DrawPic(qfalse, x+(int)(198*(chargeReady/100.f))+55, y+25, 13, 38, cgs.media.markerAscendShader);
	}
	powerLevelOffset = (Q_PrintStrlen(powerLevelString)-2)*8;
	CG_DrawSmallStringHalfHeight(x +239 -powerLevelOffset, y+46, powerLevelString, 1.f);
}

static void CG_DrawStatusBar(void){
	playerState_t		*ps;
	float				tierLast, tierNext, tier;
	clientInfo_t		*ci;
	tierConfig_cg		*activeTier;
	qboolean 			charging;
	
	ci = &cgs.clientinfo[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;
	charging = ps->weaponstate == WEAPON_CHARGING || ps->weaponstate == WEAPON_ALTCHARGING ? qtrue : qfalse;
	if(ci->lockStartTimer > cg.time)	CG_DrawPic(qfalse,0,0,640,480,cgs.media.speedLineSpinShader);
	if(ps->bitFlags & usingBoost)		CG_DrawPic(qfalse,0,0,640,480,cgs.media.speedLineShader);
	if(!cg_drawStatus.integer) return;
	tier = (float)ps->powerLevel[plTierCurrent];
	CG_CheckChat();
	CG_DrawScreenEffects();
	if(ps->lockedTarget > 0 && cgs.clientinfo[ps->lockedTarget-1].infoValid){
		playerState_t lockedTargetPS;
		lockedTargetPS.clientNum = ps->lockedTarget-1;
		lockedTargetPS.powerLevel[plCurrent] = ps->lockonData[lkPowerCurrent];
		lockedTargetPS.powerLevel[plHealth] = ps->lockonData[lkPowerHealth];
		lockedTargetPS.powerLevel[plMaximum] = ps->lockonData[lkPowerMaximum];
		lockedTargetPS.powerLevel[plFatigue] = lockedTargetPS.powerLevel[plMaximum];
		lockedTargetPS.powerLevel[plTierCurrent] = cgs.clientinfo[lockedTargetPS.clientNum].tierCurrent;
		CG_DrawHUD(ps, ps->clientNum, 0, 0, qfalse);
		CG_DrawHUD(&lockedTargetPS, lockedTargetPS.clientNum, 320, 0, qtrue);
	}
	else{
		CG_DrawHUD(ps, ps->clientNum, 0, 408, qfalse);
		if(charging) return;
		if(tier){
			activeTier = &ci->tierConfig[ci->tierCurrent];
			tierLast = 32767;
			if(activeTier->sustainCurrent && activeTier->sustainCurrent < tierLast) tierLast = (float)activeTier->sustainCurrent;
			if(activeTier->sustainFatigue && activeTier->sustainFatigue < tierLast) tierLast = (float)activeTier->sustainFatigue;
			if(activeTier->sustainHealth  && activeTier->sustainHealth  < tierLast) tierLast = (float)activeTier->sustainHealth;
			if(activeTier->sustainMaximum && activeTier->sustainMaximum < tierLast) tierLast = (float)activeTier->sustainMaximum;
			if(tierLast < 32767){
				tierLast = tierLast / (float)ps->powerLevel[plMaximum];
				CG_DrawPic(qfalse, (187*tierLast)+60, 428, 13, 38, cgs.media.markerDescendShader);
			}
		}
		if(tier < ps->powerLevel[plTierTotal]){
			activeTier = &ci->tierConfig[ci->tierCurrent+1];
			tierNext = 0;
			if(activeTier->requirementCurrent && activeTier->requirementCurrent > tierNext) tierNext = (float)activeTier->requirementCurrent;
			if(activeTier->requirementFatigue && activeTier->requirementFatigue > tierNext) tierNext = (float)activeTier->requirementFatigue;
			if(activeTier->requirementMaximum && activeTier->requirementMaximum > tierNext) tierNext = (float)activeTier->requirementMaximum;
			if(activeTier->requirementHealth  && activeTier->requirementHealth  > tierNext) tierNext = (float)activeTier->requirementHealth;
			if(tierNext){
				tierNext = tierNext / (float)ps->powerLevel[plMaximum];
				if(tierNext < 1.f)
					CG_DrawPic(qfalse, (187*tierNext)+60, 428, 13, 38, cgs.media.markerAscendShader);
			}
		}
	}
}

/*==================
CG_DrawSnapshot
==================*/
static float CG_DrawSnapshot(float y){
	char	*s;
	int		w;

	s = va("time:%i, snap:%i, cmd:%i", cg.snap->serverTime, cg.latestSnapshotNum, cgs.serverCommandSequence);
	w = CG_DrawStrlen(s) *BIGCHAR_WIDTH;
	CG_DrawBigString(635 -w, y +2, s, 1.f);
	return y +BIGCHAR_HEIGHT +4;
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	8

static float CG_DrawFPS(float y){
	char		*s;
	int			w, i, total, fps,
				t, frameTime;
	static int	index, previousTimes[FPS_FRAMES],
				previous, lastupdate;
	const int	xOffset = 0;
	
	t = trap_Milliseconds();
	frameTime = t -previous;
	previous = t;
	if(t -lastupdate > 50){
		lastupdate = t;
		previousTimes[index %FPS_FRAMES] = frameTime;
		index++;
	}
	total = 0;
	for(i=0;i<FPS_FRAMES;i++)
		total += previousTimes[i];
	if(!total) total = 1;
	fps = 1000 *FPS_FRAMES / total;
	s = va("%ifps", fps);
	w = CG_DrawStrlen(s) *BIGCHAR_WIDTH;
	CG_DrawBigString(635 -w +xOffset, y+2, s, 1.f);
	return y +BIGCHAR_HEIGHT +4;
}

/*=================
CG_DrawTimer
=================*/
static float CG_DrawTimer(float y){
	char		*s;
	int			w, msec, mins, seconds, tens;

	msec = cg.time - cgs.levelStartTime;
	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins *60;
	tens = seconds / 10;
	seconds -= tens *10;

	s = va("%i:%i%i", mins, tens, seconds );
	w = CG_DrawStrlen(s) *BIGCHAR_WIDTH;
	CG_DrawBigString(635 -w, y+2, s, 1.f);
	return y +BIGCHAR_HEIGHT +4;
}

/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight(void){
	float y = 0;

	if(cg_drawSnapshot.integer) y = CG_DrawSnapshot(y);
	if(cg_drawFPS.integer)		y = CG_DrawFPS(y);
	if(cg_drawTimer.integer)	y = CG_DrawTimer(y);
}

/*==============
CG_CenterPrint
Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint(const char *str, int y, int charWidth){
	char *s;

	Q_strncpyz(cg.centerPrint, str, sizeof(cg.centerPrint));
	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;
	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while(*s){
		if(*s == '\n') cg.centerPrintLines++;
		s++;
	}
}

/*=================
CG_DrawCrosshair
=================*/
static void CG_DrawCrosshair(void){
	playerState_t	*ps;
	clientInfo_t	*ci;
	tierConfig_cg	*tier;
	qhandle_t		hShader;
	trace_t			trace;
	float			x, y, w, h,
					f;
	int				ca;
	vec3_t			muzzle, forward, up,
					start, end;
	vec4_t			lockOnEnemyColor =	{1.f,	0,	 0, 1.f},
					lockOnAllyColor	=	{  0, 1.f,	 0, 1.f},
					chargeColor	=		{.5f, .5f, 1.f, 1.f};

	if(!cg_drawCrosshair.integer || cg.snap->ps.lockedTarget > 0) return;
	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) return;
	ci = &cgs.clientinfo[cg.snap->ps.clientNum];
	tier = &ci->tierConfig[ci->tierCurrent];
	ps = &cg.predictedPlayerState;
	if(ps->bitFlags & usingMelee) return;
	AngleVectors(ps->viewangles, forward, NULL, up);
	VectorCopy(ps->origin, muzzle);
	VectorMA(muzzle, ps->viewheight, up, muzzle);
	VectorMA(muzzle, 14, forward, muzzle);
	VectorCopy(muzzle, start);
	VectorMA(start, 131072, forward, end);
	CG_Trace(&trace, start, NULL, NULL, end, cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY);	
	if(!CG_WorldCoordToScreenCoordFloat(trace.endpos, &x, &y)) return;
	w = h = cg_crosshairSize.value *8 +8;
	f = cg.time -cg.itemPickupBlendTime;
	if(f > 0 && f < ITEM_BLOB_TIME){
		f /= ITEM_BLOB_TIME;
		w = h *= f+1;
	}
	if(cg_crosshairHealth.integer){
		vec4_t hcolor;
		
		CG_ColorForHealth(hcolor);
		trap_R_SetColor(hcolor);
	}
	else trap_R_SetColor(NULL);
	ca = cg_drawCrosshair.integer;
	if(ca < 0) ca = 0;
	hShader = cgs.media.crosshairShader[ca % NUM_CROSSHAIRS];
	if(tier->crosshair){
		hShader = tier->crosshair;
		if(ps->bitFlags & isBreakingLimit && tier->crosshairPowering)
			hShader = tier->crosshairPowering;
	}
	if((cg.crosshairClientNum > 0 && cg.crosshairClientNum <= MAX_CLIENTS) || ps->lockedTarget > 0){
		if(cgs.clientinfo[cg.crosshairClientNum].team == cg.snap->ps.persistant[PERS_TEAM] &&
			cgs.clientinfo[cg.crosshairClientNum].team != TEAM_FREE)
			trap_R_SetColor(lockOnAllyColor);	// Not working?
		else
			trap_R_SetColor(lockOnEnemyColor);
	}
	else if(cg.snap->ps.currentSkill[WPSTAT_BITFLAGS] & WPF_READY || cg.snap->ps.currentSkill[WPSTAT_ALT_BITFLAGS] & WPF_READY)
		trap_R_SetColor(chargeColor);
	else trap_R_SetColor(NULL);
	CG_DrawPic(qfalse, x -.5f *w, y -.5f *h, w, h, hShader);
	trap_R_SetColor(NULL);
}

/*=================
CG_ScanForCrosshairEntity
=================*/
static void CG_ScanForCrosshairEntity(void){
	trace_t			trace;
	vec3_t			start, end,
					minSize, maxSize,
					forward, up, muzzle;
	playerState_t	*ps;
	int				i;

	ps = &cg.predictedPlayerState;
	AngleVectors(ps->viewangles, forward, NULL, up);
	VectorCopy(ps->origin, muzzle);
	VectorMA(muzzle, ps->viewheight, up, muzzle);
	VectorMA(muzzle, 14, forward, muzzle);
	VectorCopy(muzzle, start);
	VectorMA(start, 131072, forward, end);
	for(i=0;i<3;i++){
		minSize[i] = -(float)cg_lockonDistance.value;
		maxSize[i] = -minSize[i];
	}
	CG_Trace(&trace, start, minSize, maxSize, end, cg.snap->ps.clientNum, CONTENTS_BODY);
	if(trace.entityNum >= MAX_CLIENTS){
		cg.crosshairClientNum = -1;
		return;
	}
	cg.crosshairClientNum=trace.entityNum;
	cg.crosshairClientTime=cg.time;
}

/*=====================
CG_DrawCrosshairNames
=====================*/
static void CG_DrawCrosshairNames(void){
	float		w, *color;
	char		*name;
	
	if(!cg_drawCrosshair.integer) return;
	CG_ScanForCrosshairEntity();
	if(!cg_drawCrosshairNames.integer) return;
	color = CG_FadeColor(cg.crosshairClientTime, 1000, 200);
	if(!color){
		trap_R_SetColor(NULL);
		return;
	}
	name = cgs.clientinfo[cg.crosshairClientNum].name;
	w = CG_DrawStrlen(name) *BIGCHAR_WIDTH;
	CG_DrawBigString(320 -w / 2, 170, name, color[3] *.5f);
	trap_R_SetColor(NULL);
}

/*=================
CG_DrawSpectator
=================*/
static void CG_DrawSpectator(void) {
	CG_DrawBigString(248, 440, "SPECTATOR", 1.f);
	CG_DrawBigString(224, 460, "(join world)", 1.f);
}

/*=================
CG_Draw2D
=================*/
void CG_DrawScripted2D(void){
	int i;
	overlay2D *current;

	for(i=0;i<16;++i){
		current = &cg.scripted2D[i];
		if(current->active){
			if(cg.time <= current->endTime || current->endTime == -1){
				CG_DrawPic(qfalse, current->x, current->y, current->width, current->height, current->shader);
				continue;
			}
			current->active = qfalse;
		}
	}
}

static void CG_Draw2D(void){
	// if we are taking a levelshot for the menu, don't draw anything
	if(cg.levelShot || !cg_draw2D.integer || (cg.snap->ps.pm_type == PM_INTERMISSION)) return;
	if(cg_scripted2D.integer) CG_DrawScripted2D();
	if(cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR){
		CG_DrawSpectator();
		CG_DrawCrosshair();
		CG_DrawCrosshairNames();
		CG_DrawRadar();
	}
	else if(!(cg.snap->ps.bitFlags & usingSoar))
		if(cg.snap->ps.timers[tmTransform] <= 1 && cg.snap->ps.powerups[PW_STATE] >= 0){
			CG_DrawStatusBar();
			CG_DrawCrosshair();
			CG_DrawCrosshairNames();
			CG_DrawRadar();
			if(!(cg.snap->ps.bitFlags & usingMelee)) CG_DrawWeaponSelect();
		}
	CG_DrawUpperRight();
}

void CG_DrawScreenFlash(void){
	float* color = CG_FadeColor(cg.screenFlashTime, cg.screenFlastTimeTotal, cg.screenFlashFadeTime);
	vec4_t white = {1.0f,1.0f,1.0f,0.5f};
	if(!color){return;}
	if(cg.snap->ps.timers[tmBlind] > 0){
		trap_R_SetColor(color);
		white[3] = color[3] * cg.screenFlashFadeAmount * 0.5f;
		CG_DrawRect(0,0,SCREEN_WIDTH,SCREEN_HEIGHT,SCREEN_WIDTH*SCREEN_HEIGHT,white);
		trap_R_SetColor(NULL);
	}
}

/*=====================
CG_DrawActive
Perform all drawing needed to completely fill the screen
=====================*/
void CG_DrawActive(stereoFrame_t stereoView){
	vec4_t		water = {.25f, .5f, 1.f, .1f};
	int			contents;

	if(!cg.snap){
		CG_DrawInformation();
		return;
	}
	CG_TileClear();
	trap_R_RenderScene(&cg.refdef);
	contents = CG_PointContents(cg.refdef.vieworg, -1);
	if(contents & CONTENTS_WATER){
		float phase = cg.time / 1000.0 *.2f *M_PI *2;
		water[3] = .1f +(.02f *sin(phase));
		CG_DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH*SCREEN_HEIGHT, water);
	}
	CG_DrawScreenFlash();
 	CG_Draw2D();
}
