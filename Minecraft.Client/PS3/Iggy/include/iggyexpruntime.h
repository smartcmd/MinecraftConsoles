#ifndef __RAD_INCLUDE_IGGYEXPRUNTIME_H__
#define __RAD_INCLUDE_IGGYEXPRUNTIME_H__

#include "rrCore.h"

#define IDOC

RADDEFSTART

#ifndef __RAD_HIGGYEXP_
#define __RAD_HIGGYEXP_
typedef void * HIGGYEXP;
#endif

//idoc(parent,IggyExpRuntime_API)

#define IGGYEXP_MIN_STORAGE  1024   IDOC
/* The minimum-sized block you must provide to $IggyExpCreate */

IDOC RADEXPFUNC HIGGYEXP RADEXPLINK IggyExpCreate(char *ip_address, S32 port, void *storage, S32 storage_size_in_bytes);
/* Opens a connection to $IggyExplorer and returns an $HIGGYEXP wrapping the connection.

   $:ip_address The address of the machine running Iggy Explorer (can be numeric with dots, or textual, including "localhost")
   $:port The port number on which Iggy Explorer is listening for a network connection (the default is 9190)
   $:storage A small block of storage that needed to store the $HIGGYEXP, must be at least $IGGYEXP_MIN_STORAGE
   $:storage_size_in_bytes The size of the block pointer to by <tt>storage</tt>

Returns a nullptr HIGGYEXP if the IP address/hostname can't be resolved, or no Iggy Explorer
can be contacted at the specified address/port. Otherwise returns a non-nullptr $HIGGYEXP
which you can pass to $IggyUseExplorer. */

IDOC RADEXPFUNC void  RADEXPLINK IggyExpDestroy(HIGGYEXP p);
/* Closes and destroys a connection to $IggyExplorer */

IDOC RADEXPFUNC rrbool RADEXPLINK IggyExpCheckValidity(HIGGYEXP p);
/* Checks if the connection represented by an $HIGGYEXP is still valid, i.e.
still connected to $IggyExplorer.

Returns true if the connection is still valid; returns false if it is not valid.

This might happen if someone closes Iggy Explorer, Iggy Explorer crashes, or
the network fails. You can this to poll and detect these conditions and do
something in response, such as trying to open a new connection.

An invalid $HIGGYEXP must still be shutdown with $IggyExpDestroy. */

RADDEFEND

#endif//__RAD_INCLUDE_IGGYEXPRUNTIME_H__