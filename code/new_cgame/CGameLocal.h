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

#ifndef __OG_CGAME_LOCAL_H__
#define __OG_CGAME_LOCAL_H__

#include "../cgame/cg_local.h"
#include "../Public/CGamePublic.h"

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

  ogCGameLocal

==============================================================================
*/
class ogCGameLocal : public ogCGameExport {
public:
	int GetApiVersion( void ) const { return CGAME_API_VERSION; }
	void OnLoad( og::PluginImportBase *pluginImport );
	void OnUnload( void );

	void Init( int serverMessageNum, int serverCommandSequence, int clientNum );
	void Shutdown( void );
	bool ConsoleCommand( void );
	void DrawActiveFrame( int serverTime, stereoFrame_t stereoView, bool demoPlayback );
	int  CrosshairPlayer( void );
	int  LastAttacker( void );
	void KeyEvent( int key, bool down );
	void MouseEvent( int dx, int dy );
	void EventHandling( int type );
public:
	ogCGameLocal();
};

extern ogCGameLocal cgameLocal;

#endif
