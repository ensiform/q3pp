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

#include "GameLocal.h"

ogGameLocal gameLocal;

ogGameImport *trap       = NULL;
botlib_export_t *botlib  = NULL;
cvarSystem_t *cvarSystem = NULL;

namespace og {
	og::FileSystem *FS = NULL;
}

void G_InitGame( int levelTime, int randomSeed, bool restart );
void G_RunFrame( int levelTime );
void G_ShutdownGame( bool restart );

/*
================
ogGameLocal::ogGameLocal
================
*/
ogGameLocal::ogGameLocal() {}
void ogGameLocal::OnLoad( og::PluginImportBase *pluginImport ) {
	trap       = (ogGameImport *)pluginImport;
	cvarSystem = trap->cvarSystem;
	botlib     = trap->botlib;
	og::FS     = trap->fs;

	og::CommonSetFileSystem(og::FS);
}
void ogGameLocal::OnUnload( void ) {
	cvarSystem = NULL;
	botlib     = NULL;
}
void ogGameLocal::Init( int levelTime, int randomSeed, bool restart ) {
	G_InitGame( levelTime, randomSeed, restart );
}
void ogGameLocal::Shutdown( bool restart ) {
	G_ShutdownGame( restart );
}
void ogGameLocal::RunFrame( int levelTime ) {
	G_RunFrame( levelTime );
}
char *ogGameLocal::ClientConnect( int clientNum, bool firstTime, bool isBot ) {
	return ::ClientConnect( clientNum, firstTime, isBot );
}
void ogGameLocal::ClientDisconnect( int clientNum ) {
	::ClientDisconnect( clientNum );
}
void ogGameLocal::ClientBegin( int clientNum ) {
	::ClientBegin( clientNum );
}
void ogGameLocal::ClientThink( int clientNum ) {
	::ClientThink( clientNum );
}
void ogGameLocal::ClientCommand( int clientNum ) {
	::ClientCommand( clientNum );
}
void ogGameLocal::ClientUserinfoChanged( int clientNum ) {
	::ClientUserinfoChanged( clientNum );
}
bool ogGameLocal::ConsoleCommand( void ) {
	return ::ConsoleCommand();
}
int ogGameLocal::BotAIStartFrame( int time ) {
	return ::BotAIStartFrame( time );
}
Q_EXPORT
og::PluginExportBase *getPluginExport( void ) {
	return &gameLocal;
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
