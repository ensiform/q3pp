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
// sv_game.c -- interface to the game dll

#include "server.h"

#include "../botlib/botlib.h"

botlib_export_t	*botlib_export;

ogGameExport *gameExport = NULL;
static ogGameImport gameImport;

// these functions must be used instead of pointer arithmetic, because
// the game allocates gentities with private information after the server shared part
int	SV_NumForGentity( sharedEntity_t *ent ) {
	int		num;

	num = ( (byte *)ent - (byte *)sv.gentities ) / sv.gentitySize;

	return num;
}

sharedEntity_t *SV_GentityNum( int num ) {
	sharedEntity_t *ent;

	ent = (sharedEntity_t *)((byte *)sv.gentities + sv.gentitySize*(num));

	return ent;
}

playerState_t *SV_GameClientNum( int num ) {
	playerState_t	*ps;

	ps = (playerState_t *)((byte *)sv.gameClients + sv.gameClientSize*(num));

	return ps;
}

svEntity_t	*SV_SvEntityForGentity( sharedEntity_t *gEnt ) {
	if ( !gEnt || gEnt->s.number < 0 || gEnt->s.number >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}
	return &sv.svEntities[ gEnt->s.number ];
}

sharedEntity_t *SV_GEntityForSvEntity( svEntity_t *svEnt ) {
	int		num;

	num = svEnt - sv.svEntities;
	return SV_GentityNum( num );
}

/*
===============
SV_GameSendServerCommand

Sends a command string to a client
===============
*/
void SV_GameSendServerCommand( int clientNum, const char *text ) {
	if ( clientNum == -1 ) {
		SV_SendServerCommand( NULL, "%s", text );
	} else {
		if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
			return;
		}
		SV_SendServerCommand( svs.clients + clientNum, "%s", text );	
	}
}


/*
===============
SV_GameDropClient

Disconnects the client with a message
===============
*/
void SV_GameDropClient( int clientNum, const char *reason ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		return;
	}
	SV_DropClient( svs.clients + clientNum, reason );	
}


/*
=================
SV_SetBrushModel

sets mins and maxs for inline bmodels
=================
*/
void SV_SetBrushModel( sharedEntity_t *ent, const char *name ) {
	clipHandle_t	h;
	vec3_t			mins, maxs;

	if (!name) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: NULL" );
	}

	if (name[0] != '*') {
		Com_Error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );
	}


	ent->s.modelindex = atoi( name + 1 );

	h = CM_InlineModel( ent->s.modelindex );
	CM_ModelBounds( h, mins, maxs );
	VectorCopy (mins, ent->r.mins);
	VectorCopy (maxs, ent->r.maxs);
	ent->r.bmodel = true;

	ent->r.contents = -1;		// we don't know exactly what is in the brushes

	SV_LinkEntity( ent );		// FIXME: remove
}



/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
bool SV_inPVS (const vec3_t p1, const vec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPVS (cluster);

	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);
	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return false;
	if (!CM_AreasConnected (area1, area2))
		return false;		// a door blocks sight
	return true;
}


/*
=================
SV_inPVSIgnorePortals

Does NOT check portalareas
=================
*/
bool SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2)
{
	int		leafnum;
	int		cluster;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	mask = CM_ClusterPVS (cluster);

	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);

	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return false;

	return true;
}


/*
========================
SV_AdjustAreaPortalState
========================
*/
void SV_AdjustAreaPortalState( sharedEntity_t *ent, bool open ) {
	svEntity_t	*svEnt;

	svEnt = SV_SvEntityForGentity( ent );
	if ( svEnt->areanum2 == -1 ) {
		return;
	}
	CM_AdjustAreaPortalState( svEnt->areanum, svEnt->areanum2, open );
}


/*
==================
SV_EntityContact
==================
*/
bool	SV_EntityContact( vec3_t mins, vec3_t maxs, const sharedEntity_t *gEnt, bool capsule ) {
	const float	*origin, *angles;
	clipHandle_t	ch;
	trace_t			trace;

	// check for exact collision
	origin = gEnt->r.currentOrigin;
	angles = gEnt->r.currentAngles;

	ch = SV_ClipHandleForEntity( gEnt );
	CM_TransformedBoxTrace ( &trace, vec3_origin, vec3_origin, mins, maxs,
		ch, -1, origin, angles, capsule );

	return trace.startsolid;
}


/*
===============
SV_GetServerinfo

===============
*/
void SV_GetServerinfo( char *buffer, int bufferSize ) {
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize );
	}
	Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO ), bufferSize );
}

/*
===============
SV_LocateGameData

===============
*/
void SV_LocateGameData( sharedEntity_t *gEnts, int numGEntities, int sizeofGEntity_t,
					   playerState_t *clients, int sizeofGameClient ) {
	sv.gentities = gEnts;
	sv.gentitySize = sizeofGEntity_t;
	sv.num_entities = numGEntities;

	sv.gameClients = clients;
	sv.gameClientSize = sizeofGameClient;
}


/*
===============
SV_GetUsercmd

===============
*/
void SV_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum );
	}
	*cmd = svs.clients[clientNum].lastUsercmd;
}

//==============================================

static int	FloatAsInt( float f ) {
	floatint_t fi;
	fi.f = f;
	return fi.i;
}

/*
===============
SV_ShutdownGameProgs

Called every time a map changes
===============
*/
void SV_ShutdownGameProgs( void ) {
	if ( !gameExport ) {
		return;
	}
	gameExport->Shutdown( false );
	og::Plugin::Unload( gameExport );
	gameExport = NULL;
}

/*
==================
SV_InitGameVM

Called for both a full init and a restart
==================
*/
static void SV_InitGameVM( bool restart ) {
	int		i;

	// start the entity parsing at the beginning
	sv.entityParsePoint = CM_EntityString();

	// clear all gentity pointers that might still be set from
	// a previous level
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=522
	//   now done before GAME_INIT call
	for ( i = 0 ; i < sv_maxclients->integer ; i++ ) {
		svs.clients[i].gentity = NULL;
	}
	
	// use the current msec count for a random seed
	// init for this gamestate
	gameExport->Init( sv.time, Com_Milliseconds(), restart);
}

void G_Print( const char *message ) {
	Com_Printf( "%s", message );
}
void G_Error( const char *message ) {
	Com_Error( ERR_DROP, "%s", message );
}
bool G_GetEntityToken( char *buffer, int bufferSize ) {
	const char *s = COM_Parse( &sv.entityParsePoint );
	Q_strncpyz( buffer, s, bufferSize );
	if( !sv.entityParsePoint && !s[0] ) {
		return false;
	}
	else {
		return true;
	}
}
void G_BotUserCommand( int clientNum, usercmd_t *ucmd ) {
	SV_ClientThink( &svs.clients[clientNum], ucmd );
}

void SV_AddCommand( const char *cmdName ) {
	Cmd_AddCommand( cmdName, NULL );
}

void setupGameImport( void ) {
	static cvarSystem_t local_cvarSystem;

	local_cvarSystem.Register             = Cvar_Register;
	local_cvarSystem.Update               = Cvar_Update;
	local_cvarSystem.Set                  = Cvar_SetSafe;
	local_cvarSystem.SetValue             = Cvar_SetValueSafe;
	local_cvarSystem.Reset                = Cvar_Reset;
	local_cvarSystem.VariableIntegerValue = Cvar_VariableIntegerValue;
	local_cvarSystem.VariableValue        = Cvar_VariableValue;
	local_cvarSystem.VariableStringBuffer = Cvar_VariableStringBuffer;
	local_cvarSystem.Get                  = Cvar_Get;
	local_cvarSystem.InfoStringBuffer     = Cvar_InfoStringBuffer;

	gameImport.fs                    = og::FS;
	gameImport.cvarSystem            = &local_cvarSystem;
	gameImport.botlib                = botlib_export;
	gameImport.Print                 = G_Print;
	gameImport.Error                 = G_Error;
	gameImport.Milliseconds          = Sys_Milliseconds;
	gameImport.Cmd_Argc              = Cmd_Argc;
	gameImport.Cmd_Argv              = Cmd_ArgvBuffer;
	gameImport.Cmd_Args              = Cmd_ArgsBuffer;
	gameImport.Cmd_ArgsFrom          = Cmd_ArgsFromBuffer;
	gameImport.SendConsoleCommand    = Cbuf_ExecuteText;
	gameImport.AddCommand            = SV_AddCommand;
	gameImport.RemoveCommand         = Cmd_RemoveCommandSafe;
	gameImport.LocateGameData        = SV_LocateGameData;
	gameImport.DropClient            = SV_GameDropClient;
	gameImport.SendServerCommand     = SV_GameSendServerCommand;
	gameImport.LinkEntity            = SV_LinkEntity;
	gameImport.UnlinkEntity          = SV_UnlinkEntity;
	gameImport.EntitiesInBox         = SV_AreaEntities;
	gameImport.EntityContact         = SV_EntityContact;
	gameImport.Trace                 = SV_Trace;
	gameImport.PointContents         = SV_PointContents;
	gameImport.SetBrushModel         = SV_SetBrushModel;
	gameImport.InPVS                 = SV_inPVS;
	gameImport.InPVSIgnorePortals    = SV_inPVSIgnorePortals;
	gameImport.SetConfigstring       = SV_SetConfigstring;
	gameImport.GetConfigstring       = SV_GetConfigstring;
	gameImport.SetUserinfo           = SV_SetUserinfo;
	gameImport.GetUserinfo           = SV_GetUserinfo;
	gameImport.GetServerinfo         = SV_GetServerinfo;
	gameImport.AdjustAreaPortalState = SV_AdjustAreaPortalState;
	gameImport.AreasConnected        = CM_AreasConnected;
	gameImport.BotAllocateClient     = SV_BotAllocateClient;
	gameImport.BotFreeClient         = SV_BotFreeClient;
	gameImport.GetUsercmd            = SV_GetUsercmd;
	gameImport.GetEntityToken        = G_GetEntityToken;
	gameImport.DebugPolygonCreate    = BotImport_DebugPolygonCreate;
	gameImport.DebugPolygonDelete    = BotImport_DebugPolygonDelete;
	gameImport.Com_RealTime          = Com_RealTime;
	gameImport.Sys_SnapVector        = Sys_SnapVector;
	gameImport.BotLibSetup           = SV_BotLibSetup;
	gameImport.BotLibShutdown        = SV_BotLibShutdown;
	gameImport.BotGetSnapshotEntity  = SV_BotGetSnapshotEntity;
	gameImport.BotGetConsoleMessage  = SV_BotGetConsoleMessage;
	gameImport.BotUserCommand        = G_BotUserCommand;
}

/*
===============
SV_InitGameProgs

Called on a normal map change, not on a map_restart
===============
*/
void SV_InitGameProgs( bool restart ) {
	if( restart ) {
		if( !gameExport )
			return;

		gameExport->Shutdown( true );
		og::Plugin::Unload( gameExport );
	}
	else {
		// FIXME these are temp while I make bots run in vm
		extern int bot_enable;
		cvar_t *var = Cvar_Get( "bot_enable", "1", CVAR_LATCH );
		bot_enable = var ? var->integer : 0;
	}

	setupGameImport();

	// Load the game plugin
	const char *filename = "Binaries/game" ARCH_STRING DLL_EXT;
	og::String netpath;
	if( !FS_FindDll( filename, netpath ) ) {
		Com_Error( ERR_FATAL, "Could not locate '%s'\n", filename );
	}

	gameExport = static_cast<ogGameExport *>( og::Plugin::Load( netpath.c_str(), &gameImport ) );
	if( !gameExport ) {
		Com_Error( ERR_FATAL, "og::Plugin::Load on game failed" );
	}

	SV_InitGameVM( restart );
}


/*
====================
SV_GameCommand

See if the current console command is claimed by the game
====================
*/
bool SV_GameCommand( void ) {
	if ( sv.state != SS_GAME ) {
		return false;
	}

	return gameExport->ConsoleCommand();
}

