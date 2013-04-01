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

#ifndef __OG_RENDERER_LOCAL_H__
#define __OG_RENDERER_LOCAL_H__

#ifdef RENDERER_GL2
#include "../renderergl2/tr_local.h"
#else
#include "../renderergl1/tr_local.h"
#endif
#include "../Public/RendererPublic.h"

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

  ogRendererLocal

==============================================================================
*/
class ogRendererLocal : public ogRendererExport {
public:
	int GetApiVersion( void ) const { return REF_API_VERSION; }

	void OnLoad( og::PluginImportBase *pluginImport );
	void OnUnload( void );
	
	// called before the library is unloaded
	// if the system is just reconfiguring, pass destroyWindow = false,
	// which will keep the screen from flashing to the desktop.
	void  Shutdown( bool destroyWindow );

	// All data that will be used in a level should be
	// registered before rendering any frames to prevent disk hits,
	// but they can still be registered at a later time
	// if necessary.
	//
	// BeginRegistration makes any existing media pointers invalid
	// and returns the current gl configuration, including screen width
	// and height, which can be used by the client to intelligently
	// size display elements

	void  BeginRegistration( glconfig_t *config );
	qhandle_t RegisterModel( const char *name );
	qhandle_t RegisterSkin( const char *name );
	qhandle_t RegisterShader( const char *name );
	qhandle_t RegisterShaderNoMip( const char *name );
	void  LoadWorld( const char *name );

	// the vis data is a large enough block of data that we go to the trouble
	// of sharing it with the clipmodel subsystem
	void  SetWorldVisData( const byte *vis );

	// EndRegistration will draw a tiny polygon with each texture, forcing
	// them to be loaded into card memory
	void  EndRegistration( void );

	// a scene is built up by calls to R_ClearScene and the various R_Add functions.
	// Nothing is drawn until R_RenderScene is called.
	void  ClearScene( void );
	void  AddRefEntityToScene( const refEntity_t *re );
	void  AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int num );
	int   LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );
	void  AddLightToScene( const vec3_t org, float intensity, float r, float g, float b );
	void  AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b );
	void  RenderScene( const refdef_t *fd );

	void  SetColor( const float *rgba );	// NULL = 1,1,1,1
	void  DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );	// 0 = white

	// Draw images for cinematic rendering, pass as 32 bit rgba
	void  DrawStretchRaw( int x, int y, int w, int h, int cols, int rows, const byte *data, int client, bool dirty );
	void  UploadCinematic( int w, int h, int cols, int rows, const byte *data, int client, bool dirty );

	void  BeginFrame( stereoFrame_t stereoFrame );
	// if the pointers are not NULL, timing info will be returned
	void  EndFrame( int *frontEndMsec, int *backEndMsec );
	
	int   MarkFragments( int numPoints, const vec3_t *points, const vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer );
	int   LerpTag( orientation_t *tag, qhandle_t model, int startFrame, int endFrame, float frac, const char *tagName );
	void  ModelBounds( qhandle_t model, vec3_t mins, vec3_t maxs );

	void  RegisterFont( const char *fontName, int pointSize, fontInfo_t *font );
	void  RemapShader( const char *oldShader, const char *newShader, const char *offsetTime );
	bool  GetEntityToken( char *buffer, int size );
	bool  inPVS( const vec3_t p1, const vec3_t p2 );
	
	void  TakeVideoFrame( int h, int w, byte *captureBuffer, byte *encodeBuffer, bool motionJpeg );
public:
	ogRendererLocal();
};

extern ogRendererLocal rendererLocal;

#endif
