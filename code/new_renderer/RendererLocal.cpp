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

#include "RendererLocal.h"

ogRendererLocal rendererLocal;

ogRendererImport *ri = NULL;

namespace og {
	og::FileSystem *FS = NULL;
}

/*
================
ogRendererLocal::ogRendererLocal
================
*/
ogRendererLocal::ogRendererLocal() {}
void ogRendererLocal::OnLoad( og::PluginImportBase *pluginImport ) {
	ri         = (ogRendererImport *)pluginImport;
	og::FS     = ri->fs;

	og::CommonSetFileSystem(og::FS);
}
void ogRendererLocal::OnUnload( void ) {}

void ogRendererLocal::Shutdown( bool destroyWindow ) {
	RE_Shutdown( destroyWindow );
}

void ogRendererLocal::BeginRegistration( glconfig_t *config ) {
	RE_BeginRegistration( config );
}

qhandle_t ogRendererLocal::RegisterModel( const char *name ) {
	return RE_RegisterModel( name );
}

qhandle_t ogRendererLocal::RegisterSkin( const char *name ) {
	return RE_RegisterSkin( name );
}

qhandle_t ogRendererLocal::RegisterShader( const char *name ) {
	return RE_RegisterShader( name );
}

qhandle_t ogRendererLocal::RegisterShaderNoMip( const char *name ) {
	return RE_RegisterShaderNoMip( name );
}

void ogRendererLocal::LoadWorld( const char *name ) {
	RE_LoadWorldMap( name );
}

void ogRendererLocal::SetWorldVisData( const byte *vis ) {
	RE_SetWorldVisData( vis );
}

void ogRendererLocal::EndRegistration( void ) {
	RE_EndRegistration( );
}

void ogRendererLocal::ClearScene( void ) {
	RE_ClearScene( );
}

void ogRendererLocal::AddRefEntityToScene( const refEntity_t *re ) {
	RE_AddRefEntityToScene( re );
}

void ogRendererLocal::AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int num ) {
	RE_AddPolyToScene( hShader, numVerts, verts, num );
}

int ogRendererLocal::LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir ) {
	return R_LightForPoint( point, ambientLight, directedLight, lightDir );
}

void ogRendererLocal::AddLightToScene( const vec3_t org, float intensity, float r, float g, float b ) {
	RE_AddLightToScene( org, intensity, r, g, b );
}

void ogRendererLocal::AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b ) {
	RE_AddLightToScene( org, intensity, r, g, b );
}

void ogRendererLocal::RenderScene( const refdef_t *fd ) {
	RE_RenderScene( fd );
}

void ogRendererLocal::SetColor( const float *rgba ) {
	RE_SetColor( rgba );
}

void ogRendererLocal::DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader ) {
	RE_StretchPic( x, y, w, h, s1, t1, s2, t2, hShader );
}

void ogRendererLocal::DrawStretchRaw( int x, int y, int w, int h, int cols, int rows, const byte *data, int client, bool dirty ) {
	RE_StretchRaw( x, y, w, h, cols, rows, data, client, dirty );
}

void ogRendererLocal::UploadCinematic( int w, int h, int cols, int rows, const byte *data, int client, bool dirty ) {
	RE_UploadCinematic( w, h, cols, rows, data, client, dirty );
}

void ogRendererLocal::BeginFrame( stereoFrame_t stereoFrame ) {
	RE_BeginFrame( stereoFrame );
}

void ogRendererLocal::EndFrame( int *frontEndMsec, int *backEndMsec ) {
	RE_EndFrame( frontEndMsec, backEndMsec );
}

int ogRendererLocal::MarkFragments( int numPoints, const vec3_t *points, const vec3_t projection, int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer ) {
	return R_MarkFragments( numPoints, points, projection, maxPoints, pointBuffer, maxFragments, fragmentBuffer );
}

int ogRendererLocal::LerpTag( orientation_t *tag, qhandle_t model, int startFrame, int endFrame, float frac, const char *tagName ) {
	return R_LerpTag( tag, model, startFrame, endFrame, frac, tagName );
}

void ogRendererLocal::ModelBounds( qhandle_t model, vec3_t mins, vec3_t maxs ) {
	R_ModelBounds( model, mins, maxs );
}

void ogRendererLocal::RegisterFont( const char *fontName, int pointSize, fontInfo_t *font ) {
	RE_RegisterFont( fontName, pointSize, font );
}

void ogRendererLocal::RemapShader( const char *oldShader, const char *newShader, const char *offsetTime ) {
	R_RemapShader( oldShader, newShader, offsetTime );
}

bool ogRendererLocal::GetEntityToken( char *buffer, int size ) {
	return R_GetEntityToken( buffer, size );
}

bool ogRendererLocal::inPVS( const vec3_t p1, const vec3_t p2 ) {
	return R_inPVS( p1, p2 );
}

void ogRendererLocal::TakeVideoFrame( int h, int w, byte *captureBuffer, byte *encodeBuffer, bool motionJpeg ) {
	RE_TakeVideoFrame( h, w, captureBuffer, encodeBuffer, motionJpeg );
}

Q_EXPORT
og::PluginExportBase *getPluginExport( void ) {
	return &rendererLocal;
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
