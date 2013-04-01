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

#include "client.h"

#include "../botlib/botlib.h"

extern	botlib_export_t	*botlib_export;

ogUIExport *uiExport = NULL;
static ogUIImport uiImport;

/*
====================
GetClientState
====================
*/
static void GetClientState( ogClientState *state ) {
	state->connectPacketCount = clc.connectPacketCount;
	state->connState = clc.state;
	state->serverName = clc.servername;
	state->updateInfoString = cls.updateInfoString;
	state->messageString = clc.serverMessage;
	state->clientNum = cl.snap.ps.clientNum;
}

/*
====================
LAN_LoadCachedServers
====================
*/
void LAN_LoadCachedServers( void ) {
	int size;
	cls.numglobalservers = cls.numfavoriteservers = 0;
	cls.numGlobalServerAddresses = 0;
	if( og::File * fileIn = og::FS->OpenRead( "servercache.dat" ) ) {
		cls.numglobalservers = fileIn->ReadInt();
		cls.numfavoriteservers = fileIn->ReadInt();
		size = fileIn->ReadInt();
		if (size == sizeof(cls.globalServers) + sizeof(cls.favoriteServers)) {
			fileIn->Read( &cls.globalServers, sizeof( cls.globalServers ) );
			fileIn->Read( &cls.favoriteServers, sizeof( cls.favoriteServers ) );
		} else {
			cls.numglobalservers = cls.numfavoriteservers = 0;
			cls.numGlobalServerAddresses = 0;
		}
		fileIn->Close();
	}
}

/*
====================
LAN_SaveServersToCache
====================
*/
void LAN_SaveServersToCache( void ) {
	og::File *fileOut = og::FS->OpenWrite( "servercache.dat" );
	if( fileOut ) {
		try {
			fileOut->WriteInt( cls.numglobalservers );
			fileOut->WriteInt( cls.numfavoriteservers );
			int size = sizeof( cls.globalServers ) + sizeof( cls.favoriteServers );
			fileOut->WriteInt( size );
			fileOut->Write( &cls.globalServers, sizeof( cls.globalServers ) );
			fileOut->Write( &cls.favoriteServers, sizeof( cls.favoriteServers ) );
		}
		catch( og::FileReadWriteError &err ) {
			err; // Shut up
			Com_Printf( "ERROR: error writing to servercache file\n" );
		}
		fileOut->Close();
	}
}


/*
====================
LAN_ResetPings
====================
*/
static void LAN_ResetPings(int source) {
	int count,i;
	serverInfo_t *servers = NULL;
	count = 0;

	switch (source) {
		case AS_LOCAL :
			servers = &cls.localServers[0];
			count = MAX_OTHER_SERVERS;
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			servers = &cls.globalServers[0];
			count = MAX_GLOBAL_SERVERS;
			break;
		case AS_FAVORITES :
			servers = &cls.favoriteServers[0];
			count = MAX_OTHER_SERVERS;
			break;
	}
	if (servers) {
		for (i = 0; i < count; i++) {
			servers[i].ping = -1;
		}
	}
}

/*
====================
LAN_AddServer
====================
*/
static int LAN_AddServer(int source, const char *name, const char *address) {
	int max, *count, i;
	netadr_t adr;
	serverInfo_t *servers = NULL;
	max = MAX_OTHER_SERVERS;
	count = NULL;

	switch (source) {
		case AS_LOCAL :
			count = &cls.numlocalservers;
			servers = &cls.localServers[0];
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			max = MAX_GLOBAL_SERVERS;
			count = &cls.numglobalservers;
			servers = &cls.globalServers[0];
			break;
		case AS_FAVORITES :
			count = &cls.numfavoriteservers;
			servers = &cls.favoriteServers[0];
			break;
	}
	if (servers && *count < max) {
		NET_StringToAdr( address, &adr, NA_UNSPEC );
		for ( i = 0; i < *count; i++ ) {
			if (NET_CompareAdr(servers[i].adr, adr)) {
				break;
			}
		}
		if (i >= *count) {
			servers[*count].adr = adr;
			Q_strncpyz(servers[*count].hostName, name, sizeof(servers[*count].hostName));
			servers[*count].visible = true;
			(*count)++;
			return 1;
		}
		return 0;
	}
	return -1;
}

/*
====================
LAN_RemoveServer
====================
*/
static void LAN_RemoveServer(int source, const char *addr) {
	int *count, i;
	serverInfo_t *servers = NULL;
	count = NULL;
	switch (source) {
		case AS_LOCAL :
			count = &cls.numlocalservers;
			servers = &cls.localServers[0];
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			count = &cls.numglobalservers;
			servers = &cls.globalServers[0];
			break;
		case AS_FAVORITES :
			count = &cls.numfavoriteservers;
			servers = &cls.favoriteServers[0];
			break;
	}
	if (servers) {
		netadr_t comp;
		NET_StringToAdr( addr, &comp, NA_UNSPEC );
		for (i = 0; i < *count; i++) {
			if (NET_CompareAdr( comp, servers[i].adr)) {
				int j = i;
				while (j < *count - 1) {
					Com_Memcpy(&servers[j], &servers[j+1], sizeof(servers[j]));
					j++;
				}
				(*count)--;
				break;
			}
		}
	}
}


/*
====================
LAN_GetServerCount
====================
*/
static int LAN_GetServerCount( int source ) {
	switch (source) {
		case AS_LOCAL :
			return cls.numlocalservers;
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			return cls.numglobalservers;
			break;
		case AS_FAVORITES :
			return cls.numfavoriteservers;
			break;
	}
	return 0;
}

/*
====================
LAN_GetLocalServerAddressString
====================
*/
static void LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				Q_strncpyz(buf, NET_AdrToStringwPort( cls.localServers[n].adr) , buflen );
				return;
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				Q_strncpyz(buf, NET_AdrToStringwPort( cls.globalServers[n].adr) , buflen );
				return;
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				Q_strncpyz(buf, NET_AdrToStringwPort( cls.favoriteServers[n].adr) , buflen );
				return;
			}
			break;
	}
	buf[0] = '\0';
}

/*
====================
LAN_GetServerInfo
====================
*/
static void LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	char info[MAX_STRING_CHARS];
	serverInfo_t *server = NULL;
	info[0] = '\0';
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				server = &cls.localServers[n];
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				server = &cls.globalServers[n];
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				server = &cls.favoriteServers[n];
			}
			break;
	}
	if (server && buf) {
		buf[0] = '\0';
		Info_SetValueForKey( info, "hostname", server->hostName);
		Info_SetValueForKey( info, "mapname", server->mapName);
		Info_SetValueForKey( info, "clients", va("%i",server->clients));
		Info_SetValueForKey( info, "sv_maxclients", va("%i",server->maxClients));
		Info_SetValueForKey( info, "ping", va("%i",server->ping));
		Info_SetValueForKey( info, "minping", va("%i",server->minPing));
		Info_SetValueForKey( info, "maxping", va("%i",server->maxPing));
		Info_SetValueForKey( info, "game", server->game);
		Info_SetValueForKey( info, "gametype", va("%i",server->gameType));
		Info_SetValueForKey( info, "nettype", va("%i",server->netType));
		Info_SetValueForKey( info, "addr", NET_AdrToStringwPort(server->adr));
		Info_SetValueForKey( info, "punkbuster", va("%i", server->punkbuster));
		Info_SetValueForKey( info, "g_needpass", va("%i", server->g_needpass));
		Info_SetValueForKey( info, "g_humanplayers", va("%i", server->g_humanplayers));
		Q_strncpyz(buf, info, buflen);
	} else {
		if (buf) {
			buf[0] = '\0';
		}
	}
}

/*
====================
LAN_GetServerPing
====================
*/
static int LAN_GetServerPing( int source, int n ) {
	serverInfo_t *server = NULL;
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				server = &cls.localServers[n];
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				server = &cls.globalServers[n];
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				server = &cls.favoriteServers[n];
			}
			break;
	}
	if (server) {
		return server->ping;
	}
	return -1;
}

/*
====================
LAN_GetServerPtr
====================
*/
static serverInfo_t *LAN_GetServerPtr( int source, int n ) {
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				return &cls.localServers[n];
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				return &cls.globalServers[n];
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				return &cls.favoriteServers[n];
			}
			break;
	}
	return NULL;
}

/*
====================
LAN_CompareServers
====================
*/
static int LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	int res;
	serverInfo_t *server1, *server2;

	server1 = LAN_GetServerPtr(source, s1);
	server2 = LAN_GetServerPtr(source, s2);
	if (!server1 || !server2) {
		return 0;
	}

	res = 0;
	switch( sortKey ) {
		case SORT_HOST:
			res = Q_stricmp( server1->hostName, server2->hostName );
			break;

		case SORT_MAP:
			res = Q_stricmp( server1->mapName, server2->mapName );
			break;
		case SORT_CLIENTS:
			if (server1->clients < server2->clients) {
				res = -1;
			}
			else if (server1->clients > server2->clients) {
				res = 1;
			}
			else {
				res = 0;
			}
			break;
		case SORT_GAME:
			if (server1->gameType < server2->gameType) {
				res = -1;
			}
			else if (server1->gameType > server2->gameType) {
				res = 1;
			}
			else {
				res = 0;
			}
			break;
		case SORT_PING:
			if (server1->ping < server2->ping) {
				res = -1;
			}
			else if (server1->ping > server2->ping) {
				res = 1;
			}
			else {
				res = 0;
			}
			break;
	}

	if (sortDir) {
		if (res < 0)
			return 1;
		if (res > 0)
			return -1;
		return 0;
	}
	return res;
}

/*
====================
LAN_GetPingQueueCount
====================
*/
static int LAN_GetPingQueueCount( void ) {
	return (CL_GetPingQueueCount());
}

/*
====================
LAN_ClearPing
====================
*/
static void LAN_ClearPing( int n ) {
	CL_ClearPing( n );
}

/*
====================
LAN_GetPing
====================
*/
static void LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	CL_GetPing( n, buf, buflen, pingtime );
}

/*
====================
LAN_GetPingInfo
====================
*/
static void LAN_GetPingInfo( int n, char *buf, int buflen ) {
	CL_GetPingInfo( n, buf, buflen );
}

/*
====================
LAN_MarkServerVisible
====================
*/
static void LAN_MarkServerVisible(int source, int n, bool visible ) {
	if (n == -1) {
		int count = MAX_OTHER_SERVERS;
		serverInfo_t *server = NULL;
		switch (source) {
			case AS_LOCAL :
				server = &cls.localServers[0];
				break;
			case AS_MPLAYER:
			case AS_GLOBAL :
				server = &cls.globalServers[0];
				count = MAX_GLOBAL_SERVERS;
				break;
			case AS_FAVORITES :
				server = &cls.favoriteServers[0];
				break;
		}
		if (server) {
			for (n = 0; n < count; n++) {
				server[n].visible = visible;
			}
		}

	} else {
		switch (source) {
			case AS_LOCAL :
				if (n >= 0 && n < MAX_OTHER_SERVERS) {
					cls.localServers[n].visible = visible;
				}
				break;
			case AS_MPLAYER:
			case AS_GLOBAL :
				if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
					cls.globalServers[n].visible = visible;
				}
				break;
			case AS_FAVORITES :
				if (n >= 0 && n < MAX_OTHER_SERVERS) {
					cls.favoriteServers[n].visible = visible;
				}
				break;
		}
	}
}


/*
=======================
LAN_ServerIsVisible
=======================
*/
static int LAN_ServerIsVisible(int source, int n ) {
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				return cls.localServers[n].visible;
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				return cls.globalServers[n].visible;
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				return cls.favoriteServers[n].visible;
			}
			break;
	}
	return false;
}

/*
=======================
LAN_UpdateVisiblePings
=======================
*/
bool LAN_UpdateVisiblePings(int source ) {
	return CL_UpdateVisiblePings_f(source);
}

/*
====================
LAN_GetServerStatus
====================
*/
int LAN_GetServerStatus( char *serverAddress, char *serverStatus, int maxLen ) {
	return CL_ServerStatus( serverAddress, serverStatus, maxLen );
}

/*
====================
CL_GetGlConfig
====================
*/
static void CL_GetGlconfig( glconfig_t *config ) {
	*config = cls.glconfig;
}

/*
====================
CL_GetClipboardData
====================
*/
static void CL_GetClipboardData( char *buf, int buflen ) {
	char	*cbd;

	cbd = Sys_GetClipboardData();

	if ( !cbd ) {
		*buf = 0;
		return;
	}

	Q_strncpyz( buf, cbd, buflen );

	Z_Free( cbd );
}

/*
====================
Key_KeynumToStringBuf
====================
*/
static void Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) {
	Q_strncpyz( buf, Key_KeynumToString( keynum ), buflen );
}

/*
====================
Key_GetBindingBuf
====================
*/
static void Key_GetBindingBuf( int keynum, char *buf, int buflen ) {
	char	*value;

	value = Key_GetBinding( keynum );
	if ( value ) {
		Q_strncpyz( buf, value, buflen );
	}
	else {
		*buf = 0;
	}
}

/*
====================
CLUI_GetCDKey
====================
*/
static void CLUI_GetCDKey( char *buf, int buflen ) {
#ifndef STANDALONE
	cvar_t	*fs;
	fs = Cvar_Get ("fs_game", "", CVAR_INIT|CVAR_SYSTEMINFO );
	if (UI_usesUniqueCDKey() && fs && fs->string[0] != 0) {
		Com_Memcpy( buf, &cl_cdkey[16], 16);
		buf[16] = 0;
	} else {
		Com_Memcpy( buf, cl_cdkey, 16);
		buf[16] = 0;
	}
#else
	*buf = 0;
#endif
}


/*
====================
CLUI_SetCDKey
====================
*/
#ifndef STANDALONE
static void CLUI_SetCDKey( char *buf ) {
	cvar_t	*fs;
	fs = Cvar_Get ("fs_game", "", CVAR_INIT|CVAR_SYSTEMINFO );
	if (UI_usesUniqueCDKey() && fs && fs->string[0] != 0) {
		Com_Memcpy( &cl_cdkey[16], buf, 16 );
		cl_cdkey[32] = 0;
		// set the flag so the fle will be written at the next opportunity
		cvar_modifiedFlags |= CVAR_ARCHIVE;
	} else {
		Com_Memcpy( cl_cdkey, buf, 16 );
		// set the flag so the fle will be written at the next opportunity
		cvar_modifiedFlags |= CVAR_ARCHIVE;
	}
}
#endif

/*
====================
GetConfigString
====================
*/
static int GetConfigString(int index, char *buf, int size)
{
	int		offset;

	if (index < 0 || index >= MAX_CONFIGSTRINGS)
		return false;

	offset = cl.gameState.stringOffsets[index];
	if (!offset) {
		if( size ) {
			buf[0] = 0;
		}
		return false;
	}

	Q_strncpyz( buf, cl.gameState.stringData+offset, size);
 
	return true;
}

/*
====================
FloatAsInt
====================
*/
static int FloatAsInt( float f ) {
	floatint_t fi;
	fi.f = f;
	return fi.i;
}

/*
====================
CL_ShutdownUI
====================
*/
void CL_ShutdownUI( void ) {
	Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_UI );
	cls.uiStarted = false;
	if ( !uiExport ) {
		return;
	}
	uiExport->Shutdown();
	og::Plugin::Unload( uiExport );
	uiExport = NULL;
}

void UI_Key_SetCatcher( int catcher ) {
	Key_SetCatcher( catcher | ( Key_GetCatcher() & KEYCATCH_CONSOLE ) );
}
void UI_Print( const char *message ) {
	Com_Printf( "%s", message );
}
void UI_Error( const char *message ) {
	Com_Error( ERR_DROP, "%s", message );
}
void UI_ExecuteText( int exec_when, const char *text ) {
	if( exec_when == EXEC_NOW &&
	    ( !strncmp( text, "snd_restart", 11 ) || !strncmp( text, "vid_restart", 11 ) || !strncmp( text, "quit", 5 ) || !strncmp( text, "game_restart", 12 ) ) ) {
		Com_Printf( S_COLOR_YELLOW "turning EXEC_NOW '%.11s' into EXEC_INSERT\n", text );
		exec_when = EXEC_INSERT;
	}

	Cbuf_ExecuteText( exec_when, text );
}
void UI_AddCommand( const char *cmdName ) {
	Cmd_AddCommand( cmdName, NULL );
}

extern soundInterface_t si;
void setupUiImport( void ) {
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

	uiImport.re     = re;
	uiImport.si     = &si;
	uiImport.botlib = botlib_export;

	uiImport.fs              = og::FS;
	uiImport.cvarSystem      = &local_cvarSystem;
	uiImport.Print           = UI_Print;
	uiImport.Error           = UI_Error;
	uiImport.Milliseconds    = Sys_Milliseconds;
	uiImport.Cmd_Argc        = Cmd_Argc;
	uiImport.Cmd_Argv        = Cmd_ArgvBuffer;
	uiImport.Cmd_Args        = Cmd_ArgsBuffer;
	uiImport.Cmd_ArgsFrom    = Cmd_ArgsFromBuffer;
	uiImport.Cmd_ExecuteText = UI_ExecuteText;
	uiImport.AddCommand      = UI_AddCommand;
	uiImport.RemoveCommand   = Cmd_RemoveCommandSafe;

	uiImport.Key_KeynumToStringBuf = Key_KeynumToStringBuf;
	uiImport.Key_GetBindingBuf     = Key_GetBindingBuf;
	uiImport.Key_SetBinding        = Key_SetBinding;
	uiImport.Key_IsDown            = Key_IsDown;
	uiImport.Key_GetOverstrikeMode = Key_GetOverstrikeMode;
	uiImport.Key_SetOverstrikeMode = Key_SetOverstrikeMode;
	uiImport.Key_ClearStates       = Key_ClearStates;
	uiImport.Key_GetCatcher        = Key_GetCatcher;
	uiImport.Key_SetCatcher        = UI_Key_SetCatcher;
	uiImport.GetClipboardData      = CL_GetClipboardData;
	uiImport.GetClientState        = GetClientState;
	uiImport.GetGlconfig           = CL_GetGlconfig;
	uiImport.GetConfigString       = GetConfigString;

	uiImport.UpdateScreen               = SCR_UpdateScreen;
	uiImport.LAN_LoadCachedServers      = LAN_LoadCachedServers;
	uiImport.LAN_SaveServersToCache     = LAN_SaveServersToCache;
	uiImport.LAN_AddServer              = LAN_AddServer;
	uiImport.LAN_RemoveServer           = LAN_RemoveServer;
	uiImport.LAN_GetPingQueueCount      = LAN_GetPingQueueCount;
	uiImport.LAN_ClearPing              = LAN_ClearPing;
	uiImport.LAN_GetPing                = LAN_GetPing;
	uiImport.LAN_GetPingInfo            = LAN_GetPingInfo;
	uiImport.LAN_GetServerCount         = LAN_GetServerCount;
	uiImport.LAN_GetServerAddressString = LAN_GetServerAddressString;
	uiImport.LAN_GetServerInfo          = LAN_GetServerInfo;
	uiImport.LAN_GetServerPing          = LAN_GetServerPing;
	uiImport.LAN_MarkServerVisible      = LAN_MarkServerVisible;
	uiImport.LAN_ServerIsVisible        = LAN_ServerIsVisible;
	uiImport.LAN_UpdateVisiblePings     = LAN_UpdateVisiblePings;
	uiImport.LAN_ResetPings             = LAN_ResetPings;
	uiImport.LAN_GetServerStatus        = LAN_GetServerStatus;
	uiImport.LAN_CompareServers         = LAN_CompareServers;

	uiImport.MemoryRemaining   = Hunk_MemoryRemaining;
	uiImport.GetCDKey          = CLUI_GetCDKey;
	uiImport.SetCDKey          = CLUI_SetCDKey;
	uiImport.VerifyCDKey       = CL_CDKeyValidate;
	uiImport.Com_RealTime      = Com_RealTime;
	uiImport.Sys_SnapVector    = Q_SnapVector;
	uiImport.CIN_PlayCinematic = CIN_PlayCinematic;
	uiImport.CIN_StopCinematic = CIN_StopCinematic;
	uiImport.CIN_RunCinematic  = CIN_RunCinematic;
	uiImport.CIN_DrawCinematic = CIN_DrawCinematic;
	uiImport.CIN_SetExtents    = CIN_SetExtents;
}

/*
====================
CL_InitUI
====================
*/
void CL_InitUI( void ) {
	setupUiImport();

	// Load the ui plugin
	const char *filename = "Binaries/ui" ARCH_STRING DLL_EXT;

	og::String netpath;
	if( !FS_FindDll( filename, netpath ) ) {
		Com_Error( ERR_FATAL, "Could not locate '%s'\n", filename );
	}

	uiExport = static_cast<ogUIExport *>( og::Plugin::Load( netpath.c_str(), &uiImport ) );
	if( !uiExport ) {
		Com_Error( ERR_FATAL, "og::Plugin::Load on ui failed" );
	}

	// init for this gamestate
	uiExport->Init( clc.state >= CA_CONNECTING && clc.state < CA_ACTIVE );
}


#ifndef STANDALONE
bool UI_usesUniqueCDKey( void ) {
#if 0
	if (uiExport) {
		return (VM_Call( uivm, UI_HASUNIQUECDKEY) == true);
	} else {
		return false;
	}
#endif
	return false;
}
#endif

/*
====================
UI_GameCommand

See if the current console command is claimed by the ui
====================
*/
bool UI_GameCommand( void ) {
	if ( !uiExport ) {
		return false;
	}

	return uiExport->ConsoleCommand( cls.realtime );
}
