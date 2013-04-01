/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2005 Stuart Dalton (badcdev@gmail.com)

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
#include "snd_codec.h"

/*
=================
S_ReadChunkInfo
=================
*/
static int S_ReadChunkInfo(og::File *f, char *name)
{
	name[4] = 0;

	f->Read( name, 4 );

	int len = f->ReadInt();
	if( len < 0 ) {
		Com_Printf( S_COLOR_YELLOW "WARNING: Negative chunk length\n" );
		return -1;
	}

	return len;
}

/*
=================
S_FindRIFFChunk

Returns the length of the data in the chunk, or -1 if not found
=================
*/
static int S_FindRIFFChunk( og::File *f, char *chunk ) {
	char	name[5];
	int		len;

	while( ( len = S_ReadChunkInfo(f, name) ) >= 0 )
	{
		// If this is the right chunk, return
		if( !Q_strncmp( name, chunk, 4 ) )
			return len;

		len = PAD( len, 2 );

		// Not the right chunk - skip it
		f->Seek( len, SEEK_CUR );
	}

	return -1;
}

/*
=================
S_ByteSwapRawSamples
=================
*/
static void S_ByteSwapRawSamples( int samples, int width, int s_channels, const byte *data ) {
	int		i;

	if ( width != 2 ) {
		return;
	}
	if ( LittleShort( 256 ) == 256 ) {
		return;
	}

	if ( s_channels == 2 ) {
		samples <<= 1;
	}
	for ( i = 0 ; i < samples ; i++ ) {
		((short *)data)[i] = LittleShort( ((short *)data)[i] );
	}
}

/*
=================
S_ReadRIFFHeader
=================
*/
static bool S_ReadRIFFHeader(og::File *file, snd_info_t *info)
{
	char dump[16];
	int bits;
	int fmtlen = 0;

	// skip the riff wav header
	file->Read( dump, 12 );

	// Scan for the format chunk
	if((fmtlen = S_FindRIFFChunk(file, "fmt ")) < 0)
	{
		Com_Printf( S_COLOR_RED "ERROR: Couldn't find \"fmt\" chunk\n");
		return false;
	}

	// Save the parameters
	file->ReadShort(); // wav_format
	info->channels = file->ReadShort();
	info->rate = file->ReadInt();
	file->ReadInt();
	file->ReadShort();
	bits = file->ReadShort();

	if( bits < 8 )
	{
	  Com_Printf( S_COLOR_RED "ERROR: Less than 8 bit sound is not supported\n");
	  return false;
	}

	info->width = bits / 8;
	info->dataofs = 0;

	// Skip the rest of the format chunk if required
	if(fmtlen > 16)
	{
		fmtlen -= 16;
		file->Seek( fmtlen, SEEK_CUR );
	}

	// Scan for the data chunk
	if( (info->size = S_FindRIFFChunk(file, "data")) < 0)
	{
		Com_Printf( S_COLOR_RED "ERROR: Couldn't find \"data\" chunk\n");
		return false;
	}
	info->samples = (info->size / info->width) / info->channels;

	return true;
}

// WAV codec
snd_codec_t wav_codec =
{
	"wav",
	S_WAV_CodecLoad,
	S_WAV_CodecOpenStream,
	S_WAV_CodecReadStream,
	S_WAV_CodecCloseStream,
	NULL
};

/*
=================
S_WAV_CodecLoad
=================
*/
void *S_WAV_CodecLoad(const char *filename, snd_info_t *info)
{
	// Try to open the file
	og::File * file = og::FS->OpenRead( filename );
	if(!file)
	{
		return NULL;
	}

	void *buffer;
	try {
		// Read the RIFF header
		if(!S_ReadRIFFHeader(file, info))
		{
			file->Close();
			Com_Printf( S_COLOR_RED "ERROR: Incorrect/unsupported format in \"%s\"\n",
					filename);
			return NULL;
		}

		// Allocate some memory
		buffer = Hunk_AllocateTempMemory(info->size);
		if(!buffer)
		{
			file->Close();
			Com_Printf( S_COLOR_RED "ERROR: Out of memory reading \"%s\"\n",
					filename);
			return NULL;
		}

		// Read, byteswap
		file->Read( buffer, info->size );
		S_ByteSwapRawSamples(info->samples, info->width, info->channels, (byte *)buffer);

		// Close and return
		file->Close();
		return buffer;
	}
	catch( og::FileReadWriteError &err ) {
		err; // Shut up
		file->Close();
		Com_Printf( S_COLOR_RED "Error reading wav file: %s\n", filename );
		return NULL;
	}
}

/*
=================
S_WAV_CodecOpenStream
=================
*/
snd_stream_t *S_WAV_CodecOpenStream(const char *filename)
{
	snd_stream_t *rv;

	// Open
	rv = S_CodecUtilOpen(filename, &wav_codec);
	if(!rv)
		return NULL;

	// Read the RIFF header
	if(!S_ReadRIFFHeader(rv->file, &rv->info))
	{
		S_CodecUtilClose(&rv);
		return NULL;
	}

	return rv;
}

/*
=================
S_WAV_CodecCloseStream
=================
*/
void S_WAV_CodecCloseStream(snd_stream_t *stream)
{
	S_CodecUtilClose(&stream);
}

/*
=================
S_WAV_CodecReadStream
=================
*/
int S_WAV_CodecReadStream(snd_stream_t *stream, int bytes, void *buffer)
{
	int remaining = stream->info.size - stream->pos;
	int samples;

	if(remaining <= 0)
		return 0;
	if(bytes > remaining)
		bytes = remaining;
	stream->pos += bytes;
	samples = (bytes / stream->info.width) / stream->info.channels;
	stream->file->Read( buffer, bytes );
	S_ByteSwapRawSamples(samples, stream->info.width, stream->info.channels, (byte *)buffer);
	return bytes;
}
