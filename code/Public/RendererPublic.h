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

#ifndef __OG_RENDERER_PUBLIC_H__
#define __OG_RENDERER_PUBLIC_H__

#include <og/Plugin.h>

#include <og/FileSystem.h>

#include "../renderercommon/tr_types.h"

const int REF_API_VERSION = 1; // 8

/*
==============================================================================

  ogRendererImport

==============================================================================
*/
class ogRendererImport : public og::PluginImportBase {
public:
	int GetApiVersion( void ) const { return REF_API_VERSION; }

	og::FileSystem *fs;

	// todo: make this non-c
	void ( *Printf )( int printLevel, const char *message, ... ) __attribute__ ((format (printf, 2, 3)));
	void ( *Error )( int errorLevel, const char *message, ... ) __attribute__ ((noreturn, format (printf, 2, 3)));

	int ( *Milliseconds )( void );
	
	// stack based memory allocation for per-level things that
	// won't be freed
#ifdef HUNK_DEBUG
	void * ( *Hunk_AllocDebug )( int size, ha_pref pref, char *label, char *file, int line );
#else
	void * ( *Hunk_Alloc )( int size, ha_pref pref );
#endif
	void * ( *Hunk_AllocateTempMemory)( int size );
	void ( *Hunk_FreeTempMemory )( void *block );

	// dynamic memory allocator for things that need to be freed
	void * ( *Malloc )( int bytes );
	void ( *Free )( void *buf );

	cvar_t	*(*Cvar_Get)( const char *name, const char *value, int flags );
	void	(*Cvar_Set)( const char *name, const char *value );
	void	(*Cvar_SetValue) (const char *name, float value);
	void	(*Cvar_CheckRange)( cvar_t *cv, float minVal, float maxVal, bool shouldBeIntegral );

	int		(*Cvar_VariableIntegerValue) (const char *var_name);
	
	void ( *Cmd_AddCommand )( const char *cmdName, void(*cmd)(void) );
	void ( *Cmd_RemoveCommand )( const char *cmd_name );

	int ( *Cmd_Argc )( void );
	char * ( *Cmd_Argv )( int arg );
	
	void ( *Cmd_ExecuteText )( int exec_when, const char *text );
	
	byte * ( *CM_ClusterPVS )( int cluster );

	// visualization for debugging collision detection
	void ( *CM_DrawDebugSurface )( void (*drawPoly)(int color, int numPoints, float *points) );
	
	// cinematic stuff
	void ( *CIN_UploadCinematic )( int handle );
	int ( *CIN_PlayCinematic )( const char *arg0, int xpos, int ypos, int width, int height, int bits );
	e_status ( *CIN_RunCinematic )( int handle );

	void ( *CL_WriteAVIVideoFrame )( const byte *buffer, int size );

	// input event handling
	void ( *IN_Init )( void );
	void ( *IN_Shutdown )( void );
	void ( *IN_Restart )( void );

	// math
	long ( *ftol )( float f );

	// system stuff
	void ( *Sys_SetEnv )( const char *name, const char *value );
	void ( *Sys_GLimpSafeInit )( void );
	void ( *Sys_GLimpInit )( void );
	bool ( *Sys_LowPhysicalMemory )( void );
};

/*
==============================================================================

  ogRendererExport

==============================================================================
*/
class ogRendererExport : public og::PluginExportBase {
public:
	// called before the library is unloaded
	// if the system is just reconfiguring, pass destroyWindow = false,
	// which will keep the screen from flashing to the desktop.
	virtual void  Shutdown( bool destroyWindow )        = 0;

	// All data that will be used in a level should be
	// registered before rendering any frames to prevent disk hits,
	// but they can still be registered at a later time
	// if necessary.
	//
	// BeginRegistration makes any existing media pointers invalid
	// and returns the current gl configuration, including screen width
	// and height, which can be used by the client to intelligently
	// size display elements

	virtual void  BeginRegistration( glconfig_t *config ) = 0;
	virtual qhandle_t RegisterModel( const char *name ) = 0;
	virtual qhandle_t RegisterSkin( const char *name ) = 0;
	virtual qhandle_t RegisterShader( const char *name ) = 0;
	virtual qhandle_t RegisterShaderNoMip( const char *name ) = 0;
	virtual void  LoadWorld( const char *name ) = 0;

	// the vis data is a large enough block of data that we go to the trouble
	// of sharing it with the clipmodel subsystem
	virtual void  SetWorldVisData( const byte *vis ) = 0;

	// EndRegistration will draw a tiny polygon with each texture, forcing
	// them to be loaded into card memory
	virtual void  EndRegistration( void ) = 0;

	// a scene is built up by calls to R_ClearScene and the various R_Add functions.
	// Nothing is drawn until R_RenderScene is called.
	virtual void  ClearScene( void ) = 0;
	virtual void  AddRefEntityToScene( const refEntity_t *re ) = 0;
	virtual void  AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int num ) = 0;
	virtual int   LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir ) = 0;
	virtual void  AddLightToScene( const vec3_t org, float intensity, float r, float g, float b ) = 0;
	virtual void  AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b ) = 0;
	virtual void  RenderScene( const refdef_t *fd ) = 0;

	virtual void  SetColor( const float *rgba ) = 0;	// NULL = 1,1,1,1
	virtual void  DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader ) = 0;	// 0 = white

	// Draw images for cinematic rendering, pass as 32 bit rgba
	virtual void  DrawStretchRaw( int x, int y, int w, int h, int cols, int rows, const byte *data, int client, bool dirty ) = 0;
	virtual void  UploadCinematic( int w, int h, int cols, int rows, const byte *data, int client, bool dirty ) = 0;

	virtual void  BeginFrame( stereoFrame_t stereoFrame ) = 0;
	// if the pointers are not NULL, timing info will be returned
	virtual void  EndFrame( int *frontEndMsec, int *backEndMsec ) = 0;
	
	virtual int   MarkFragments( int numPoints, const vec3_t *points, const vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer ) = 0;
	virtual int   LerpTag( orientation_t *tag,  qhandle_t model, int startFrame, int endFrame, float frac, const char *tagName ) = 0;
	virtual void  ModelBounds( qhandle_t model, vec3_t mins, vec3_t maxs ) = 0;

	virtual void  RegisterFont( const char *fontName, int pointSize, fontInfo_t *font ) = 0;
	virtual void  RemapShader( const char *oldShader, const char *newShader, const char *offsetTime ) = 0;
	virtual bool  GetEntityToken( char *buffer, int size ) = 0;
	virtual bool  inPVS( const vec3_t p1, const vec3_t p2 ) = 0;
	
	virtual void  TakeVideoFrame( int h, int w, byte* captureBuffer, byte *encodeBuffer, bool motionJpeg ) = 0;
};

#endif
