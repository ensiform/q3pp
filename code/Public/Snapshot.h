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

#ifndef __OG_SNAPSHOT_H__
#define __OG_SNAPSHOT_H__

const int MAX_ENTITIES_IN_SNAPSHOT = 256;

/*
==============================================================================

  ogSnapshot

  Snapshots are a view of the server at a given time.
  Snapshots are generated at regular time intervals by the server,
  but they may not be sent if a client's rate level is exceeded, or
  they may be dropped by the network.
==============================================================================
*/
class ogSnapshot {
public:
	int snapFlags;                      // SNAPFLAG_RATE_DELAYED, etc
	int ping;

	int serverTime;                     // server time the message is valid for (in msec)

	byte areamask[MAX_MAP_AREA_BYTES];          // portalarea visibility bits

	playerState_t ps;                   // complete information about the current player at this time

	int numEntities;                                        // all of the entities that need to be presented
	entityState_t entities[MAX_ENTITIES_IN_SNAPSHOT];       // at the time of this snapshot

	int numServerCommands;                  // text based server commands to execute when this
	int serverCommandSequence;              // snapshot becomes current
};

#endif
