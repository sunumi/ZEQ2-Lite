//cg_tiers.c: Client-side tier file system.
#include "cg_local.h"
qboolean CG_RegisterClientModelnameWithTiers(clientInfo_t *ci, const char *modelName, const char *skinName){
	int	index,partIndex,damageIndex,lastSkinIndex,lastModelIndex,i;
	char filename[MAX_QPATH*2];
	char tierPath[MAX_QPATH];
	char tempPath[MAX_QPATH];
	char legsPath[MAX_QPATH];
	char headPath[MAX_QPATH];
	char cameraPath[MAX_QPATH];
	char upperPrefix[] = "upper_";
	char lowerPrefix[] = "lower_";
	qhandle_t tempShader;
	Com_sprintf(legsPath,sizeof(legsPath),"%s",modelName);
	Com_sprintf(headPath,sizeof(headPath),"%s",modelName);
	Com_sprintf(cameraPath,sizeof(cameraPath),"%s",modelName);
	Com_sprintf(tempPath,sizeof(tempPath),"players/%s/animation.cfg",ci->legsModelName);
	if(ci->legsModelName && trap_FS_FOpenFile(tempPath,0,FS_READ)>0){Com_sprintf(legsPath,sizeof(legsPath),"%s",ci->legsModelName);}
	Com_sprintf(tempPath,sizeof(tempPath),"players/%s/animation.cfg",ci->headModelName);
	if(ci->headModelName && trap_FS_FOpenFile(tempPath,0,FS_READ)>0){Com_sprintf(headPath,sizeof(headPath),"%s",ci->headModelName);}
	Com_sprintf(filename,sizeof(filename),"players/%s/animation.cfg",modelName);
	if(!CG_ParseAnimationFile(filename,ci,qtrue)){
		//error out
	}
	for(i=0;i<8;i++){
		// ===================================
		// Config
		// ===================================
		Com_sprintf(tierPath,sizeof(tierPath),"players/%s/tier%i/icon.png",modelName,i+1);
		if(trap_FS_FOpenFile(tierPath,0,FS_READ)<=0){continue;}
		memset(&ci->tierConfig[i],0,sizeof(tierConfig_cg));
		Com_sprintf(tierPath,sizeof(tierPath),"players/%s/tier%i/",modelName,i+1);
		tempShader = trap_R_RegisterShaderNoMip(strcat(tierPath,"icon.png"));
		for(index=0;index<10;++index){
			ci->tierConfig[i].icon2D[index] = tempShader; 
			ci->tierConfig[i].screenEffect[index] = cgs.media.clearShader;
		}
		Com_sprintf(tierPath,sizeof(tierPath),"players/%s/tier%i/",modelName,i+1);
		if(trap_FS_FOpenFile(strcat(tierPath,"transformFirst.opus"),0,FS_READ)>0){
			Com_sprintf(tierPath,sizeof(tierPath),"players/%s/tier%i/",modelName,i+1);
			ci->tierConfig[i].soundTransformFirst = trap_S_RegisterSound(strcat(tierPath,"transformFirst.opus"),qfalse);
		}
		Com_sprintf(tierPath,sizeof(tierPath),"players/%s/tier%i/",modelName,i+1);
		if(trap_FS_FOpenFile(strcat(tierPath,"transformUp.opus"),0,FS_READ)>0){
			Com_sprintf(tierPath,sizeof(tierPath),"players/%s/tier%i/",modelName,i+1);
			ci->tierConfig[i].soundTransformUp = trap_S_RegisterSound(strcat(tierPath,"transformUp.opus"),qfalse);
		}
		Com_sprintf(tierPath,sizeof(tierPath),"players/%s/tier%i/",modelName,i+1);
		if(trap_FS_FOpenFile(strcat(tierPath,"transformDown.opus"),0,FS_READ)>0){
			Com_sprintf(tierPath,sizeof(tierPath),"players/%s/tier%i/",modelName,i+1);
			ci->tierConfig[i].soundTransformDown = trap_S_RegisterSound(strcat(tierPath,"transformDown.opus"),qfalse);
		}
		Com_sprintf(tierPath,sizeof(tierPath),"players/%s/tier%i/",modelName,i+1);
		if(trap_FS_FOpenFile(strcat(tierPath,"poweringUp.opus"),0,FS_READ)>0){
			Com_sprintf(tierPath,sizeof(tierPath),"players/%s/tier%i/",modelName,i+1);
			ci->tierConfig[i].soundPoweringUp = trap_S_RegisterSound(strcat(tierPath,"poweringUp.opus"),qfalse);
		}
		Com_sprintf(tierPath,sizeof(tierPath),"players/%s/tier%i/transformScript.cfg",modelName,i+1);
		if(trap_FS_FOpenFile(tierPath,0,FS_READ)>0){
			ci->tierConfig[i].transformScriptExists = qtrue;
		}
		Com_sprintf(filename,sizeof(filename),"players/tierDefault.cfg");
		parseTier(filename,&ci->tierConfig[i]);
		Com_sprintf(filename,sizeof(filename),"players/%s/tier%i/tier.cfg",modelName,i+1);
		parseTier(filename,&ci->tierConfig[i]);
		// ===================================
		// Models
		// ===================================
		Com_sprintf(filename,sizeof(filename),"players/%s/tier%i/lower.iqm",legsPath,i+1);
		if(trap_FS_FOpenFile(filename,0,FS_READ)>0){ci->legsModel[i] = trap_R_RegisterModel(filename);}
		else{
			Com_sprintf(filename,sizeof(filename),"players/%s/tier%i/lower.md3",legsPath,i+1);
			if(trap_FS_FOpenFile(filename,0,FS_READ)>0){ci->legsModel[i] = trap_R_RegisterModel(filename);}
			else if(i!=0){ci->legsModel[i] = ci->legsModel[i-1];}
			else{return qfalse;}
		}
		Com_sprintf(filename,sizeof(filename),"players/%s/tier%i/upper.iqm",modelName,i+1);
		if(trap_FS_FOpenFile(filename,0,FS_READ)>0){ci->torsoModel[i] = trap_R_RegisterModel(filename);}
		else{
			Com_sprintf(filename,sizeof(filename),"players/%s/tier%i/upper.md3",modelName,i+1);
			if(trap_FS_FOpenFile(filename,0,FS_READ)>0){ci->torsoModel[i] = trap_R_RegisterModel(filename);}
			else if(i!=0){ci->torsoModel[i] = ci->torsoModel[i-1];}
			else{return qfalse;}
		}
		Com_sprintf(filename,sizeof(filename),"players/%s/tier%i/head.iqm",headPath,i+1);
		if(trap_FS_FOpenFile(filename,0,FS_READ)>0){ci->headModel[i] = trap_R_RegisterModel(filename);}
		else{
			Com_sprintf(filename,sizeof(filename),"players/%s/tier%i/head.md3",headPath,i+1);
			if(trap_FS_FOpenFile(filename,0,FS_READ)>0){ci->headModel[i] = trap_R_RegisterModel(filename);}
			else if(i!=0){ci->headModel[i] = ci->headModel[i-1];}
			else{return qfalse;}
		}
		Com_sprintf(filename, sizeof(filename), "players/%s/tier%i/camera.md3", modelName, i+1);
		if(trap_FS_FOpenFile(filename,0,FS_READ)>0){ci->cameraModel[i] = trap_R_RegisterModel(filename);}
		else if(i!=0){ci->cameraModel[i] = ci->cameraModel[i-1];}
		else if(!i && trap_FS_FOpenFile(filename,0,FS_READ)<1){ci->cameraModel[i] = trap_R_RegisterModel("players/cameraDefault.md3");}
		// ===================================
		// Skins
		// ===================================
		Com_sprintf(filename,sizeof(filename),"players/%s/tier%i/%s.skin",legsPath,i+1,skinName);
		if(trap_FS_FOpenFile(filename,0,FS_READ)>0){ci->legsSkin[i] = trap_R_RegisterSkin(filename);}
		else if(i!=0){ci->legsSkin[i] = ci->legsSkin[i-1];}
		else{return qfalse;}
		Com_sprintf(filename,sizeof(filename),"players/%s/tier%i/%s.skin",modelName,i+1,skinName);
		if(trap_FS_FOpenFile(filename,0,FS_READ)>0){ci->torsoSkin[i] = trap_R_RegisterSkin(filename);}
		else if(i!=0){ci->torsoSkin[i] = ci->torsoSkin[i-1];}	
		else{return qfalse;}
		Com_sprintf(filename, sizeof(filename), "players/%s/tier%i/%s.skin", headPath, i+1,skinName);
		if(trap_FS_FOpenFile(filename,0,FS_READ)>0){ci->headSkin[i] = trap_R_RegisterSkin(filename);}
		else if(i!=0){ci->headSkin[i] = ci->headSkin[i-1];}
		// ===================================
		// Damage States
		// ===================================
		for(partIndex=0;partIndex<3;++partIndex){
			lastSkinIndex = -1;
			lastModelIndex = -1;
			for(damageIndex=9;damageIndex>=0;damageIndex--){
				if(partIndex==0){
					Com_sprintf(filename, sizeof(filename),"players/%s/tier%i/head_percent%i.iqm",headPath,i+1,(damageIndex+1)*10);
					if(trap_FS_FOpenFile(filename,0,FS_READ)>0){
						ci->modelDamageState[i][partIndex][damageIndex] = trap_R_RegisterModel(filename);
						lastModelIndex = damageIndex;
					}
					else{
						Com_sprintf(filename, sizeof(filename),"players/%s/tier%i/head_percent%i.md3",headPath,i+1,(damageIndex+1)*10);
						if(trap_FS_FOpenFile(filename,0,FS_READ)>0){
							ci->modelDamageState[i][partIndex][damageIndex] = trap_R_RegisterModel(filename);
							lastModelIndex = damageIndex;
						}
						else{ci->modelDamageState[i][partIndex][damageIndex] = lastModelIndex != -1 ? ci->modelDamageState[i][partIndex][lastModelIndex] : ci->headModel[i];}
					}
					Com_sprintf(filename, sizeof(filename), "players/%s/tier%i/%s_percent%i.skin",headPath,i+1,skinName,(damageIndex+1)*10);
					if(trap_FS_FOpenFile(filename,0,FS_READ)>0){
						ci->skinDamageState[i][partIndex][damageIndex] = trap_R_RegisterSkin(filename);
						lastSkinIndex = damageIndex;
					}
					else{ci->skinDamageState[i][partIndex][damageIndex] = lastSkinIndex != -1 ? ci->skinDamageState[i][partIndex][lastSkinIndex] : ci->headSkin[i];}
				}
				else if(partIndex==1){
					Com_sprintf(filename, sizeof(filename), "players/%s/tier%i/upper_percent%i.md3",modelName,i+1,(damageIndex+1)*10);
					if(trap_FS_FOpenFile(filename,0,FS_READ)>0){
						ci->modelDamageState[i][partIndex][damageIndex] = trap_R_RegisterModel(filename);
						lastModelIndex = damageIndex;
					}
					else{ci->modelDamageState[i][partIndex][damageIndex] = lastModelIndex != -1? ci->modelDamageState[i][partIndex][lastModelIndex] : ci->torsoModel[i];}
					Com_sprintf(filename, sizeof(filename), "players/%s/tier%i/%s%s_percent%i.skin",modelName,i+1,upperPrefix,skinName,(damageIndex+1)*10);
					if(trap_FS_FOpenFile(filename,0,FS_READ)>0){
						ci->skinDamageState[i][partIndex][damageIndex] = trap_R_RegisterSkin(filename);
						lastSkinIndex = damageIndex;
					}
					else{ci->skinDamageState[i][partIndex][damageIndex] = lastSkinIndex != -1 ? ci->skinDamageState[i][partIndex][lastSkinIndex] : ci->torsoSkin[i];}
				}
				else if(partIndex==2){
					Com_sprintf(filename, sizeof(filename), "players/%s/tier%i/lower_percent%i.md3",legsPath,i+1,(damageIndex+1)*10);
					if(trap_FS_FOpenFile(filename,0,FS_READ)>0){
						ci->modelDamageState[i][partIndex][damageIndex] = trap_R_RegisterModel(filename);
						lastModelIndex = damageIndex;
					}
					else{ci->modelDamageState[i][partIndex][damageIndex] = lastModelIndex != -1 ? ci->modelDamageState[i][partIndex][lastModelIndex] : ci->legsModel[i];}
					Com_sprintf(filename, sizeof(filename), "players/%s/tier%i/%s%s_percent%i.skin",legsPath,i+1,lowerPrefix,skinName,(damageIndex+1)*10);
					if(trap_FS_FOpenFile(filename,0,FS_READ)>0){
						ci->skinDamageState[i][partIndex][damageIndex] = trap_R_RegisterSkin(filename);
						lastSkinIndex = damageIndex;
					}
					else{ci->skinDamageState[i][partIndex][damageIndex] = lastSkinIndex != -1 ? ci->skinDamageState[i][partIndex][lastSkinIndex] : ci->legsSkin[i];}
				}
			}
		}
	}
	return qtrue;
}
void parseTier(char *path,tierConfig_cg *tier){
	fileHandle_t tierCFG;
	char *token,*parse;
	int fileLength;
	int tokenInt;
	char fileContents[32000];
	if(trap_FS_FOpenFile(path,0,FS_READ)>0){
		fileLength = trap_FS_FOpenFile(path,&tierCFG,FS_READ);
		trap_FS_Read(fileContents,fileLength,tierCFG);
		fileContents[fileLength] = 0;
		trap_FS_FCloseFile(tierCFG);
		parse = fileContents;
		while(1){
			token = COM_Parse(&parse);
			if(!token[0]){break;}
			else if(!Q_stricmp(token,"tierName")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				Q_strncpyz(tier->name,token,sizeof(tier->name));
			}
			else if(!Q_stricmp(token,"powerLevelHudMultiplier")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->hudMultiplier = atof(token);
			}
			else if(!Q_stricmp(token,"sustainCurrent")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->sustainCurrent = atoi(token);
			}
			else if(!Q_stricmp(token,"sustainCurrentPercent")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->sustainCurrentPercent = atoi(token);
			}
			else if(!Q_stricmp(token,"sustainMaximum")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->sustainMaximum = atoi(token);
			}
			else if(!Q_stricmp(token,"sustainFatigue")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->sustainFatigue = atoi(token);
			}
			else if(!Q_stricmp(token,"sustainHealth")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->sustainHealth = atoi(token);
			}
			else if(!Q_stricmp(token,"requirementCurrent")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->requirementCurrent = atoi(token);
			}
			else if(!Q_stricmp(token,"requirementCurrentPercent")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->requirementCurrentPercent = atoi(token);
			}
			else if(!Q_stricmp(token,"requirementFatigue")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->requirementFatigue = atoi(token);
			}
			else if(!Q_stricmp(token,"requirementMaximum")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->requirementMaximum = atoi(token);
			}
			else if(!Q_stricmp(token,"requirementHealth")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->requirementHealth = atoi(token);
			}
			else if(!Q_stricmp(token,"requirementHealthMaximum")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->requirementHealthMaximum = atoi(token);
			}
			else if(!Q_stricmp(token,"transformSoundFirst")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->soundTransformFirst = trap_S_RegisterSound(token,qfalse);
			}
			else if(!Q_stricmp(token,"transformSoundUp")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->soundTransformUp = trap_S_RegisterSound(token,qfalse);
			}
			else if(!Q_stricmp(token,"transformSoundDown")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->soundTransformDown = trap_S_RegisterSound(token,qfalse);
			}
			else if(!Q_stricmp(token,"transformMusic")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				if(trap_FS_FOpenFile(va("music/%s",token),0,FS_READ)>0){
					Q_strncpyz(tier->transformMusic,token,sizeof(tier->transformMusic));
					token = COM_Parse(&parse);
					if(!token[0]){break;}
					tier->transformMusicLength = CG_GetMilliseconds(token);
				}
			}
			else if(!Q_stricmp(token,"poweringUpSound")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->soundPoweringUp = trap_S_RegisterSound(token,qfalse);
			}
			else if(!Q_stricmp(token,"damageFeatures")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->damageFeatures = strlen(token) == 4 ? qtrue : qfalse;
			}
			else if(!Q_stricmp(token,"damageModelsRevertHealed")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->damageModelsRevertHealed = strlen(token) == 4 ? qtrue : qfalse;
			}
			else if(!Q_stricmp(token,"damageTexturesRevertHealed")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->damageTexturesRevertHealed = strlen(token) == 4 ? qtrue : qfalse;
			}
			else if(!Q_stricmp(token,"icon2DShader")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tokenInt = atoi(token);
				token = COM_Parse(&parse);
				if(Q_stricmp(token,"default")){
					int countdown = tokenInt/10-1;
					while(countdown > 0){
						tier->icon2D[countdown] = trap_R_RegisterShaderNoMip(token);
						countdown -= 1;
					}
				}
			}
			else if(!Q_stricmp(token,"icon2DPoweringShader")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				if(Q_stricmp(token,"default")){
					tier->icon2DPowering = trap_R_RegisterShaderNoMip(token);
				}
			}
			else if(!Q_stricmp(token,"icon2DTransformingShader")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				if(Q_stricmp(token,"default")){
					tier->icon2DTransforming = trap_R_RegisterShaderNoMip(token);
				}
			}
			else if(!Q_stricmp(token,"icon3DOffset")){
				tier->icon3DOffset[0] = atoi(COM_Parse(&parse));
				tier->icon3DOffset[1] = atoi(COM_Parse(&parse));
			}
			else if(!Q_stricmp(token,"icon3DRotation")){
				tier->icon3DRotation[0] = atoi(COM_Parse(&parse));
				tier->icon3DRotation[1] = atoi(COM_Parse(&parse));
				tier->icon3DRotation[2] = atoi(COM_Parse(&parse));
			}
			else if(!Q_stricmp(token,"icon3DSize")){
				tier->icon3DSize[0] = atoi(COM_Parse(&parse));
				tier->icon3DSize[1] = atoi(COM_Parse(&parse));
			}
			else if(!Q_stricmp(token,"icon3DZoom")){
				tier->icon3DZoom = atof(COM_Parse(&parse));
			}
			else if(!Q_stricmp(token,"screenShader")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tokenInt = atoi(token);
				token = COM_Parse(&parse);
				if(Q_stricmp(token,"default")){
					int countdown = tokenInt/10-1;
					while(countdown > 0){
						tier->screenEffect[countdown] = trap_R_RegisterShaderNoMip(token);
						countdown -= 1;
					}
				}
			}
			else if(!Q_stricmp(token,"screenPoweringShader")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				if(Q_stricmp(token,"default")){
					tier->screenEffectPowering = trap_R_RegisterShaderNoMip(token);
				}
			}
			else if(!Q_stricmp(token,"screenTransformingShader")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				if(Q_stricmp(token,"default")){
					tier->screenEffectTransforming = trap_R_RegisterShaderNoMip(token);
				}
			}
			else if(!Q_stricmp(token,"crosshairShader")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				if(Q_stricmp(token,"default")){
					tier->crosshair = trap_R_RegisterShaderNoMip(token);
				}
			}
			else if(!Q_stricmp(token,"crosshairPoweringShader")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				if(Q_stricmp(token,"default")){
					tier->crosshairPowering = trap_R_RegisterShaderNoMip(token);
				}
			}
			else if(!Q_stricmp(token,"meshScale")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->meshScale = atof(token);
			}
			else if(!Q_stricmp(token,"meshOffset")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->meshOffset = atoi(token);
			}
			else if(!Q_stricmp(token,"cameraOffsetSlide")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->cameraOffset[0] = atoi(token);
			}
			else if(!Q_stricmp(token,"cameraOffsetHeight")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->cameraOffset[1] = atoi(token);
			}
			else if(!Q_stricmp(token,"cameraOffsetRange")){
				token = COM_Parse(&parse);
				if(!token[0]){break;}
				tier->cameraOffset[2] = atoi(token);
			}
		}
	}
}
void CG_NextTier_f(void){
	if(!cg.snap || (cg.snap->ps.pm_flags & PMF_FOLLOW)){return;}
	if(cg.tierSelect == -1){
		cg.tierSelectionMode = 2;
		cg.tierSelect = 0;
	}
}
void CG_PrevTier_f(void){
	if(!cg.snap || (cg.snap->ps.pm_flags & PMF_FOLLOW)){return;}
	if(cg.tierSelect == -1){
		cg.tierSelectionMode = 1;
		cg.tierSelect = 0;
	}
}
void CG_Tier_f(void){
	int num;
	if(!cg.snap || (cg.snap->ps.pm_flags & PMF_FOLLOW)){return;}
	num = atoi(CG_Argv(1));
	if(num < 0 || num > 10) {return;}
	if(cg.tierSelect == -1){
		cg.tierSelect = num;
		cg.tierSelectionMode = 3;
	}
}
