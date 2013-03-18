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

#include "UILocal.h"

ogUILocal uiLocal;

ogUIImport *trap         = NULL;
botlib_export_t *botlib  = NULL;
cvarSystem_t *cvarSystem = NULL;

namespace og {
	og::FileSystem *FS = NULL;
}
/*
================
ogUILocal::ogUILocal
================
*/
ogUILocal::ogUILocal() {}
void ogUILocal::OnLoad( og::PluginImportBase *pluginImport ) {
	trap       = (ogUIImport *)pluginImport;
	cvarSystem = trap->cvarSystem;
	botlib     = trap->botlib;
	og::FS     = trap->fs;

	og::CommonSetFileSystem(og::FS);
}
void ogUILocal::OnUnload( void ) {
	cvarSystem = NULL;
	botlib     = NULL;
}
void ogUILocal::Init( bool inGameLoad ) {
	UI_Init( inGameLoad );
}
void ogUILocal::Shutdown( void ) {
	UI_Shutdown();
}
void ogUILocal::KeyEvent( int key, bool down ) {
	UI_KeyEvent( key, down );
}
void ogUILocal::MouseEvent( int dx, int dy ) {
	UI_MouseEvent( dx, dy );
}
void ogUILocal::Refresh( int realTime ) {
	UI_Refresh( realTime );
}
bool ogUILocal::IsFullscreen( void ) {
	return UI_IsFullscreen();
}
void ogUILocal::SetActiveMenu( uiMenuCommand_t menu ) {
	UI_SetActiveMenu( menu );
}
bool ogUILocal::ConsoleCommand( int realTime ) {
	return UI_ConsoleCommand( realTime );
}
void ogUILocal::DrawConnectScreen( bool overlay ) {
	UI_DrawConnectScreen( overlay );
}
Q_EXPORT
og::PluginExportBase *getPluginExport( void ) {
	return &uiLocal;
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
