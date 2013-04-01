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

#ifndef __OG_UI_PUBLIC_H__
#define __OG_UI_PUBLIC_H__

#include <og/Plugin.h>

#include "../client/snd_local.h"
#include "../Public/RendererPublic.h"
//#include "../renderercommon/tr_public.h"
#include "../botlib/botlib.h"

const int UI_API_VERSION = 1;

class ogClientState {
public:
	connstate_t connState;
	int connectPacketCount;
	int clientNum;
	og::String serverName;
	og::String updateInfoString;
	og::String messageString;
};

typedef enum {
	UIMENU_NONE,
	UIMENU_MAIN,
	UIMENU_INGAME,
	UIMENU_NEED_CD,
	UIMENU_BAD_CD_KEY,
	UIMENU_TEAM,
	UIMENU_POSTGAME
} uiMenuCommand_t;

typedef enum {
	SORT_HOST,
	SORT_MAP,
	SORT_CLIENTS,
	SORT_GAME,
	SORT_PING,
	SORT_PUNKBUSTER
} uiSortKey_t;

/*
==============================================================================

  ogUIImport

==============================================================================
*/
class ogUIImport : public og::PluginImportBase {
public:
	int GetApiVersion( void ) const { return UI_API_VERSION; }

	og::FileSystem *fs;
	cvarSystem_t *cvarSystem;
	ogRendererExport *re;
	//refexport_t *re;
	soundInterface_t *si;
	botlib_export_t *botlib;

	// todo: make this non-c
	void ( *Print )( const char *message );
	void ( *Error )( const char *message ) __attribute__ ( ( noreturn ) );
	int ( *Milliseconds )( void );
	int ( *Cmd_Argc )( void );
	void ( *Cmd_Argv )( int arg, char *buffer, int bufferLength );
	void ( *Cmd_Args )( char *buffer, int bufferLength );
	void ( *Cmd_ArgsFrom )( int arg, char *buffer, int bufferLength );
	void ( *Cmd_ExecuteText )( int exec_when, const char *text );
	void ( *AddCommand )( const char *cmdName );
	void ( *RemoveCommand )( const char *cmd_name );
	void ( *Key_KeynumToStringBuf )( int keynum, char *buf, int buflen );
	void ( *Key_GetBindingBuf )( int keynum, char *buf, int buflen );
	void ( *Key_SetBinding )( int keynum, const char *binding );
	bool ( *Key_IsDown )( int keynum );
	bool ( *Key_GetOverstrikeMode )( void );
	void ( *Key_SetOverstrikeMode )( bool state );
	void ( *Key_ClearStates )( void );
	int ( *Key_GetCatcher )( void );
	void ( *Key_SetCatcher )( int catcher );
	void ( *GetClipboardData )( char *buf, int buflen );
	void ( *GetClientState )( ogClientState *state );
	void ( *GetGlconfig )( glconfig_t *config );
	int ( *GetConfigString )( int index, char *buf, int size );
	void ( *UpdateScreen )( void );
	void ( *LAN_LoadCachedServers )( void );
	void ( *LAN_SaveServersToCache )( void );
	int ( *LAN_AddServer )( int source, const char *name, const char *address );
	void ( *LAN_RemoveServer )( int source, const char *addr );
	int ( *LAN_GetPingQueueCount )( void );
	void ( *LAN_ClearPing )( int n );
	void ( *LAN_GetPing )( int n, char *buf, int buflen, int *pingtime );
	void ( *LAN_GetPingInfo )( int n, char *buf, int buflen );
	int ( *LAN_GetServerCount )( int source );
	void ( *LAN_GetServerAddressString )( int source, int n, char *buf, int buflen );
	void ( *LAN_GetServerInfo )( int source, int n, char *buf, int buflen );
	int ( *LAN_GetServerPing )( int source, int n );
	void ( *LAN_MarkServerVisible )( int source, int n, bool visible );
	int ( *LAN_ServerIsVisible )( int source, int n );
	bool ( *LAN_UpdateVisiblePings )( int source );
	void ( *LAN_ResetPings )( int source );
	int ( *LAN_GetServerStatus )( char *serverAddress, char *serverStatus, int maxLen );
	int ( *LAN_CompareServers )( int source, int sortKey, int sortDir, int s1, int s2 );
	int ( *MemoryRemaining )( void );
	void ( *GetCDKey )( char *buf, int buflen );
	void ( *SetCDKey )( char *buf );
	bool ( *VerifyCDKey )( const char *key, const char *checksum );
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

  ogUIExport

==============================================================================
*/
class ogUIExport : public og::PluginExportBase {
public:
	virtual void Init( bool inGameLoad )               = 0;
	virtual void Shutdown( void )                      = 0;
	virtual void KeyEvent( int key, bool down )        = 0;
	virtual void MouseEvent( int dx, int dy )          = 0;
	virtual void Refresh( int realTime )               = 0;
	virtual bool IsFullscreen( void )                  = 0;
	virtual void SetActiveMenu( uiMenuCommand_t menu ) = 0;
	virtual bool ConsoleCommand( int realTime )        = 0;
	virtual void DrawConnectScreen( bool overlay )     = 0;
};

#endif
