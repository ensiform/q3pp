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

#ifndef __OG_GAME_PUBLIC_H__
#define __OG_GAME_PUBLIC_H__

#include <og/Plugin.h>

#include "../botlib/botlib.h"
#include "../qcommon/qfiles.h"

const int GAME_API_VERSION = 1;

/*
==============================================================================

  ogGameImport

==============================================================================
*/
class ogGameImport : public og::PluginImportBase {
public:
	int GetApiVersion( void ) const { return GAME_API_VERSION; }

	og::FileSystem *fs;
	cvarSystem_t *cvarSystem;
	botlib_export_t *botlib;

	// todo: make this non-c
	void ( *Print )( const char *message );
	void ( *Error )( const char *message ) __attribute__ ( ( noreturn ) );
	int ( *Milliseconds )( void );
	int ( *Cmd_Argc )( void );
	void ( *Cmd_Argv )( int arg, char *buffer, int bufferLength );
	void ( *Cmd_Args )( char *buffer, int bufferLength );
	void ( *Cmd_ArgsFrom )( int arg, char *buffer, int bufferLength );
	void ( *SendConsoleCommand )( int exec_when, const char *text );
	void ( *AddCommand )( const char *cmdName );
	void ( *RemoveCommand )( const char *cmd_name );
	void ( *LocateGameData )( sharedEntity_t *gEnts, int numGEntities, int sizeofGEntity_t,
				  playerState_t *clients, int sizeofGameClient );
	void ( *DropClient )( int clientNum, const char *reason );
	void ( *SendServerCommand )( int clientNum, const char *text );
	void ( *LinkEntity )( sharedEntity_t *ent );
	void ( *UnlinkEntity )( sharedEntity_t *ent );
	int ( *EntitiesInBox )( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
	bool ( *EntityContact )( vec3_t mins, vec3_t maxs, const sharedEntity_t *gEnt, bool capsule );
	void ( *Trace )( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, bool capsule );
	int ( *PointContents )( const vec3_t p, int passEntityNum );
	void ( *SetBrushModel )( sharedEntity_t *ent, const char *name );
	bool ( *InPVS )( const vec3_t p1, const vec3_t p2 );
	bool ( *InPVSIgnorePortals )( const vec3_t p1, const vec3_t p2 );
	void ( *SetConfigstring )( int index, const char *val );
	void ( *GetConfigstring )( int index, char *buffer, int bufferSize );
	void ( *SetUserinfo )( int index, const char *val );
	void ( *GetUserinfo )( int index, char *buffer, int bufferSize );
	void ( *GetServerinfo )( char *buffer, int bufferSize );
	void ( *AdjustAreaPortalState )( sharedEntity_t *ent, bool open );
	bool ( *AreasConnected )( int area1, int area2 );
	int ( *BotAllocateClient )( void );
	void ( *BotFreeClient )( int clientNum );
	void ( *GetUsercmd )( int clientNum, usercmd_t *cmd );
	bool ( *GetEntityToken )( char *buffer, int bufferSize );
	int ( *DebugPolygonCreate )( int color, int numPoints, vec3_t *points );
	void ( *DebugPolygonDelete )( int id );
	int ( *Com_RealTime )( qtime_t *qtime );
	void ( *Sys_SnapVector )( float *v );
	int ( *BotLibSetup )( void );
	int ( *BotLibShutdown )( void );
	int ( *BotGetSnapshotEntity )( int client, int sequence );
	int ( *BotGetConsoleMessage )( int client, char *buf, int size );
	void ( *BotUserCommand )( int clientNum, usercmd_t *ucmd );
};

/*
==============================================================================

  ogGameExport

==============================================================================
*/
class ogGameExport : public og::PluginExportBase {
public:
	virtual void  Init( int levelTime, int randomSeed, bool restart )        = 0;
	virtual void  Shutdown( bool restart )                                   = 0;
	virtual char *ClientConnect( int clientNum, bool firstTime, bool isBot ) = 0;
	virtual void  ClientThink( int clientNum )                               = 0;
	virtual void  ClientUserinfoChanged( int clientNum )                     = 0;
	virtual void  ClientDisconnect( int clientNum )                          = 0;
	virtual void  ClientBegin( int clientNum )                               = 0;
	virtual void  ClientCommand( int clientNum )                             = 0;
	virtual void  RunFrame( int levelTime )                                  = 0;
	virtual bool  ConsoleCommand( void )                                     = 0;
	virtual int   BotAIStartFrame( int time )                                = 0;
};

#endif
