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

#ifndef __OG_GAME_LOCAL_H__
#define __OG_GAME_LOCAL_H__

#include "../game/g_local.h"
#include "../Public/GamePublic.h"

/*
================
ogError

class to throw errors
================
*/
class ogError {
private:
	bool isFatal;
	og::String error;
public:
	ogError( const char *err, bool fatal = false ) { isFatal = fatal;
							 error   = err; }

	OG_INLINE const char *getMessage( void ) {
		return error.c_str();
	}
	OG_INLINE bool IsFatal( void ) { return isFatal; }
};

/*
==============================================================================

  ogGameLocal

==============================================================================
*/
class ogGameLocal : public ogGameExport {
public:
	int GetApiVersion( void ) const { return GAME_API_VERSION; }

	void  OnLoad( og::PluginImportBase *pluginImport );
	void  OnUnload( void );
	void  Init( int levelTime, int randomSeed, bool restart );
	void  Shutdown( bool restart );
	char *ClientConnect( int clientNum, bool firstTime, bool isBot );
	void  ClientThink( int clientNum );
	void  ClientUserinfoChanged( int clientNum );
	void  ClientDisconnect( int clientNum );
	void  ClientBegin( int clientNum );
	void  ClientCommand( int clientNum );
	void  RunFrame( int levelTime );
	bool  ConsoleCommand( void );
	int   BotAIStartFrame( int time );
public:
	ogGameLocal();
};

extern ogGameLocal gameLocal;

#endif
