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

#include "CGameLocal.h"

ogCGameLocal cgameLocal;

ogCGameImport *trap      = NULL;
botlib_export_t *botlib  = NULL;
cvarSystem_t *cvarSystem = NULL;
namespace og {
	og::FileSystem *FS = NULL;
}

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );

/*
================
ogCGameLocal::ogCGameLocal
================
*/
ogCGameLocal::ogCGameLocal() {}
void ogCGameLocal::OnLoad( og::PluginImportBase *pluginImport ) {
	trap       = (ogCGameImport *)pluginImport;
	cvarSystem = trap->cvarSystem;
	botlib     = trap->botlib;
	og::FS     = trap->fs;

	og::CommonSetFileSystem(og::FS);
}
void ogCGameLocal::OnUnload( void ) {
	cvarSystem = NULL;
	botlib     = NULL;
}
void ogCGameLocal::Init( int serverMessageNum, int serverCommandSequence, int clientNum ) {
	CG_Init( serverMessageNum, serverCommandSequence, clientNum );
}
void ogCGameLocal::Shutdown( void ) {
	CG_Shutdown();
}
bool ogCGameLocal::ConsoleCommand( void ) {
	return CG_ConsoleCommand();
}
void ogCGameLocal::DrawActiveFrame( int serverTime, stereoFrame_t stereoView, bool demoPlayback ) {
	CG_DrawActiveFrame( serverTime, stereoView, demoPlayback );
}
int ogCGameLocal::CrosshairPlayer( void ) {
	return CG_CrosshairPlayer();
}
int ogCGameLocal::LastAttacker( void ) {
	return CG_LastAttacker();
}
void ogCGameLocal::KeyEvent( int key, bool down ) {
	CG_KeyEvent( key, down );
}
void ogCGameLocal::MouseEvent( int dx, int dy ) {
	CG_MouseEvent( dx, dy );
}
void ogCGameLocal::EventHandling( int type ) {
	CG_EventHandling( type );
}
Q_EXPORT
og::PluginExportBase *getPluginExport( void ) {
	return &cgameLocal;
}
namespace og {
	namespace User {
		void Error( og::ErrorId id, const char *msg, const char *param ) {
			// Todo: Throw an exception on the id's you think are important.
			og::String error;
			CreateErrorString( id, msg, param, error );
			Com_Printf( S_COLOR_RED "ERROR: %s\n", error.c_str() );
		}
		void Warning( const char *msg ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: %s\n", msg );
		}
		void AssertFailed( const char *code, const char *function ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: Assert (%s) failed in %s!\n", code, function );
		}
	}
}
