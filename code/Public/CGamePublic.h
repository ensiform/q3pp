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

#ifndef __OG_CGAME_PUBLIC_H__
#define __OG_CGAME_PUBLIC_H__

#include <og/Plugin.h>

#include "../botlib/botlib.h"
#include "../client/snd_local.h"
#include "../renderercommon/tr_public.h"
#include "../game/bg_public.h"
#include "Snapshot.h"

const int CGAME_API_VERSION = 1;

// allow a lot of command backups for very fast systems
// multiple commands may be combined into a single packet, so this
// needs to be larger than PACKET_BACKUP
const int CMD_BACKUP = 64;
const int CMD_MASK   = CMD_BACKUP - 1;

enum {
	CGAME_EVENT_NONE,
	CGAME_EVENT_TEAMMENU,
	CGAME_EVENT_SCOREBOARD,
	CGAME_EVENT_EDITHUD
};

/*
==============================================================================

  ogCGameImport

==============================================================================
*/
class ogCGameImport : public og::PluginImportBase {
public:
	int GetApiVersion( void ) const { return CGAME_API_VERSION; }

	og::FileSystem *fs;
	refexport_t *re;
	soundInterface_t *si;
	botlib_export_t *botlib;
	cvarSystem_t *cvarSystem;

	// todo: make this non-c
	void ( *Print )( const char *message );
	void ( *Error )( const char *message ) __attribute__ ( ( noreturn ) );
	int ( *Milliseconds )( void );
	int ( *Cmd_Argc )( void );
	void ( *Cmd_Argv )( int arg, char *buffer, int bufferLength );
	void ( *Cmd_Args )( char *buffer, int bufferLength );
	void ( *Cmd_ArgsFrom )( int arg, char *buffer, int bufferLength );
	void ( *SendConsoleCommand )( const char *text );
	void ( *AddCommand )( const char *cmdName );
	void ( *RemoveCommand )( const char *cmd_name );
	void ( *SendClientCommand )( const char *cmd, bool isDisconnectCmd );
	void ( *UpdateScreen )( void );
	void ( *CM_LoadMap )( const char *mapname );
	int ( *CM_NumInlineModels )( void );
	clipHandle_t ( *CM_InlineModel )( int index );
	clipHandle_t ( *CM_TempBoxModel )( const vec3_t mins, const vec3_t maxs, bool capsule );
	int ( *CM_PointContents )( const vec3_t p, clipHandle_t model );
	int ( *CM_TransformedPointContents )( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
	void ( *CM_BoxTrace )( trace_t *results, const vec3_t start, const vec3_t end,
			       const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask, bool capsule );
	void ( *CM_TransformedBoxTrace )( trace_t *results, const vec3_t start, const vec3_t end,
					  const vec3_t mins, const vec3_t maxs, clipHandle_t model, int brushmask,
					  const vec3_t origin, const vec3_t angles, bool capsule );

	void ( *GetGlconfig )( glconfig_t *config );
	void ( *GetGameState )( gameState_t *gs );
	void ( *GetCurrentSnapshotNumber )( int *snapshotNumber, int *serverTime );
	bool ( *GetSnapshot )( int snapshotNumber, ogSnapshot *snapshot );
	bool ( *GetServerCommand )( int serverCommandNumber );
	int ( *GetCurrentCmdNumber )( void );
	bool ( *GetUserCmd )( int cmdNumber, usercmd_t *ucmd );
	void ( *SetUserCmdValue )( int userCmdValue, float sensitivityScale );
	int ( *MemoryRemaining )( void );
	bool ( *Key_IsDown )( int keynum );
	int ( *Key_GetCatcher )( void );
	void ( *Key_SetCatcher )( int catcher );
	int ( *Key_GetKey )( const char *binding );
	int ( *Com_RealTime )( qtime_t *qtime );
	void ( *Sys_SnapVector )( float *v );
	int ( *CIN_PlayCinematic )( const char *arg0, int xpos, int ypos, int width, int height, int bits );
	e_status ( *CIN_StopCinematic )( int handle );
	e_status ( *CIN_RunCinematic )( int handle );
	void ( *CIN_DrawCinematic )( int handle );
	void ( *CIN_SetExtents )( int handle, int x, int y, int w, int h );
};

/*
==============================================================================

  ogCGameExport

==============================================================================
*/
class ogCGameExport : public og::PluginExportBase {
public:
	virtual void Init( int serverMessageNum, int serverCommandSequence, int clientNum )         = 0;
	virtual void Shutdown( void )                                                               = 0;
	virtual bool ConsoleCommand( void )                                                         = 0;
	virtual void DrawActiveFrame( int serverTime, stereoFrame_t stereoView, bool demoPlayback ) = 0;
	virtual int  CrosshairPlayer( void )                                                        = 0;
	virtual int  LastAttacker( void )                                                           = 0;
	virtual void KeyEvent( int key, bool down )                                                 = 0;
	virtual void MouseEvent( int dx, int dy )                                                   = 0;
	virtual void EventHandling( int type )                                                      = 0;
};

#endif
