// gdraw_shared.inl - author: Sean Barrett - copyright 2010 RAD Game Tools
//
// This file implements some common code that can be shared across
// all the sample implementations of GDraw.

#ifdef IGGY_DISABLE_GDRAW_ASSERT
#define assert(x)
#else
#include <assert.h>
#endif

#ifndef GDRAW_MAYBE_UNUSED
#define GDRAW_MAYBE_UNUSED
#endif

///////////////////////////////////////////////////////////////
//
// GDrawHandleCache manages resource "handles" used by Iggy
// (i.e. these handles wrap the platform resource handles,
// and this file provides those wrappers and facilities for
// LRU tracking them). Moreover, for console platforms, we
// actually implement our own managed resource pools.
//
// This is the main state machine when GDRAW_MANAGE_MEM is defined:
// (which covers all console platforms)
//
//       +------+         +--------+             |
//       | Live |<------->| Locked |             |
//       +------+         +--------+             |
//       /     \                  ^              |
//      /       \                  \             |
//     v         v                  \            |
// +------+    +------+    +------+  |           |
// | Dead |--->| Free |<---| User |  |           |
// +------+    +------+    +------+  |           |
//     ^         ^    ^       ^      |           |
//      \       /       \     |      |           |
//       \     /          v   |      |           |
//     +--------+         +-------+ /            |
//     | Pinned |<--------| Alloc |/             |
//     +--------+         +-------+              |
//
// "Free" handles are not in use and available for allocation.
// "Alloc" handles have been assigned by GDraw, but do not yet
//   have a system resource backing them. Resources stay in
//   this state until we know that for sure that we're going
//   to be able to successfully complete creation, at which
//   point the resource transitions to one of the regular states.
// "Live" handles correspond to resources that may be used
//   for rendering. They are kept in LRU order. Old resources
//   may be evicted to make space.
// "Locked" handles cover resources that are going to be used
//   in the next draw command. Once a resource is marked locked,
//   it may not be evicted until it's back to "Live".
// "Dead" handles describe resources that have been freed on the
//   CPU side, but are still in use by the GPU. Their memory may
//   only be reclaimed once the GPU is done with them, at which
//   point they are moved to the "Free" list. Items on the "Dead"
//   list appear ordered by the last time they were used by the
//   GPU - "most stale" first.
// "Pinned" resources can be used in any draw call without getting
//   locked first. They can never be LRU-freed, but their memory
//   is still managed by GDraw. Currently this is only used for
//   the Iggy font cache.
// "User" (user-owned) resources are exactly that. They act much like
//   pinned resources, but their memory isn't managed by GDraw.
//   When a user-owned resource is freed, we really need to free
//   it immediately (instead of marking it as "dead"), which might
//   necessitate stalling the CPU until the GPU is finished using
//   that resource. Since we don't own the memory, delayed frees
//   are not an option.
//
// Without GDRAW_MANAGE_MEM, there's no "Dead" resources, and all
// frees are performed immediately.

typedef struct GDrawHandleCache GDrawHandleCache;
typedef struct GDrawHandle GDrawHandle;

typedef struct
{
   U64 value;
} GDrawFence;

typedef enum
{
   GDRAW_HANDLE_STATE_free = 0,
   GDRAW_HANDLE_STATE_live,
   GDRAW_HANDLE_STATE_locked,
   GDRAW_HANDLE_STATE_dead,
   GDRAW_HANDLE_STATE_pinned,
   GDRAW_HANDLE_STATE_user_owned,
   GDRAW_HANDLE_STATE_alloc,
   GDRAW_HANDLE_STATE__count,

   // not an actual state!
   GDRAW_HANDLE_STATE_sentinel = GDRAW_HANDLE_STATE__count,
} GDrawHandleState;

struct GDrawHandle
{
   GDrawNativeHandle   handle;  // platform handle to a resource (variable size)
   void              * owner;   // 4/8 // opaque handle used to allow freeing resources without calling back to owner

   GDrawHandleCache  * cache;   // 4/8 // which cache this handle came from

   GDrawHandle       * next,*prev;  // 8/16 // doubly-linked list

   #ifdef GDRAW_MANAGE_MEM
   void              * raw_ptr; // 4/8 // pointer to allocation - when you're managing memory manually
   #ifdef GDRAW_CORRUPTION_CHECK
   U32                 cached_raw_value[4];
   rrbool              has_check_value;
   #endif
   #endif

   GDrawFence          fence;   // 8 // (optional) platform fence for resource
   // 4
   U32                 bytes:28;       // estimated storage cost to allow setting a loose limit
   U32                 state:4;        // state the handle is in
};

// validate alignment to make sure structure will pack correctly
#ifdef __RAD64__
RR_COMPILER_ASSERT((sizeof(GDrawHandle) & 7) == 0);
#else
RR_COMPILER_ASSERT((sizeof(GDrawHandle) & 3) == 0);
#endif

struct GDrawHandleCache
{
   S32          bytes_free;
   S32          total_bytes;
   S32          max_handles;
   U32          is_vertex : 1;      // vertex buffers have different warning codes and generate discard callbacks
   U32          is_thrashing : 1;
   U32          did_defragment : 1;
   // 30 unused bits
   GDrawHandle  state[GDRAW_HANDLE_STATE__count];  // sentinel nodes for all of the state lists
   #ifdef GDRAW_MANAGE_MEM
   struct gfx_allocator *alloc;
   #endif
   #ifdef GDRAW_MANAGE_MEM_TWOPOOL
   struct gfx_allocator *alloc_other;
   #endif
   GDrawFence   prev_frame_start, prev_frame_end; // fence value at start/end of previous frame, for thrashing detection
   GDrawHandle  handle[1]; // the rest of the handles must be stored right after this in the containing structure
};

#ifdef GDRAW_CORRUPTION_CHECK
// values for corruption checking
#define GDRAW_CORRUPTIONCHECK_renderbegin         0x10
#define GDRAW_CORRUPTIONCHECK_renderend           0x20
#define GDRAW_CORRUPTIONCHECK_nomoregdraw         0x30
#define GDRAW_CORRUPTIONCHECK_maketexbegin        0x40
#define GDRAW_CORRUPTIONCHECK_maketexend          0x50

#define GDRAW_CORRUPTIONCHECK_wrappedcreateend       0x60
#define GDRAW_CORRUPTIONCHECK_wrappedcreatebegin     0x61
#define GDRAW_CORRUPTIONCHECK_wrappeddestroyend      0x70
#define GDRAW_CORRUPTIONCHECK_wrappeddestroybegin    0x71

#define GDRAW_CORRUPTIONCHECK_allochandle             0x80
#define GDRAW_CORRUPTIONCHECK_allochandle_begin       0x81
#define GDRAW_CORRUPTIONCHECK_allochandle_postreap    0x82
#define GDRAW_CORRUPTIONCHECK_allochandle_postfree1   0x83
#define GDRAW_CORRUPTIONCHECK_allochandle_postfree2   0x84
#define GDRAW_CORRUPTIONCHECK_allochandle_postfree3   0x85
#define GDRAW_CORRUPTIONCHECK_allochandle_postalloc1  0x86
#define GDRAW_CORRUPTIONCHECK_allochandle_postalloc2  0x87
#define GDRAW_CORRUPTIONCHECK_allochandle_postalloc3  0x88
#define GDRAW_CORRUPTIONCHECK_allochandle_defrag      0x89

#define GDRAW_CORRUPTIONCHECK_freetex           0x90

static U32 *debug_raw_address(GDrawHandle *t, int choice)
{
   static int offset_table[4] = { 0x555555, 0xaaaaaa, 0x333333, 0x6e6e6e };
   U8 *base = (U8 *) t->raw_ptr;
   int offset = offset_table[choice] & (t->bytes-1) & ~3;
   return (U32 *) (base + offset);
}

static void debug_check_overlap_one(GDrawHandle *t, U8 *ptr, S32 len)
{
   assert(len >= 0);
   if (t->raw_ptr && t->raw_ptr != ptr) {
      assert(t->raw_ptr < ptr || t->raw_ptr >= ptr+len);  
   }
}

static void debug_check_overlap(GDrawHandleCache *c, U8 *ptr, S32 len)
{
   GDrawHandle *t = c->head;
   while (t) {
      debug_check_overlap_one(t, ptr, len);
      t = t->next;
   }
   t = c->active;
   while (t) {
      debug_check_overlap_one(t, ptr, len);
      t = t->next;
   }
}

static void debug_check_raw_values(GDrawHandleCache *c)
{
   GDrawHandle *t = c->head;
   while (t) {
      if (t->raw_ptr && t->has_check_value) {
         int i;
         for (i=0; i < 4; ++i) {
            if (*debug_raw_address(t, i) != t->cached_raw_value[i]) {
               //zlog("!Iggy texture corruption found\n");
               //zlog("t=%p, t->raw_ptr=%p\n", t, t->raw_ptr);
               //zlog("Cached values: %08x %08x %08x %08x\n", t->cached_raw_value[0], t->cached_raw_value[1], t->cached_raw_value[2], t->cached_raw_value[3]);
               //zlog("Current values: %08x %08x %08x %08x\n", *debug_raw_address(t,0), *debug_raw_address(t,1), *debug_raw_address(t,2), *debug_raw_address(t,3));
               assert(0);
            }
         }
         #if 0
         GDrawHandle *s;
         check_block_alloc(c->alloc, t->raw_ptr, 1);
         s = c->head;
         while (s != t) {
            assert(s->raw_ptr != t->raw_ptr);
            s = s->next;
         }
         s = c->active;
         while (s != nullptr) {
            assert(s->raw_ptr != t->raw_ptr);
            s = s->next;
         }
         #endif
      }
      t = t->next;
   }
   t = c->active;
   while (t) {
      if (t->raw_ptr && t->has_check_value) {
         int i;
         for (i=0; i < 4; ++i) {
            if (*debug_raw_address(t, i) != t->cached_raw_value[i]) {
               //zlog("!Iggy texture corruption found\n");
               //zlog("t=%p, t->raw_ptr=%p\n", t, t->raw_ptr);
               //zlog("Cached values: %08x %08x %08x %08x\n", t->cached_raw_value[0], t->cached_raw_value[1], t->cached_raw_value[2], t->cached_raw_value[3]);
               //zlog("Current values: %08x %08x %08x %08x\n", *debug_raw_address(t,0), *debug_raw_address(t,1), *debug_raw_address(t,2), *debug_raw_address(t,3));
               assert(0);
            }
         }
         #if 0
         GDrawHandle *s;
         check_block_alloc(c->alloc, t->raw_ptr, 1);
         s = c->active;
         while (s != t) {
            assert(s->raw_ptr != t->raw_ptr);
            s = s->next;
         }
         #endif
      }
      t = t->next;
   }
}

#ifndef GDRAW_CORRUPTION_MASK
#define GDRAW_CORRUPTION_MASK 0
#endif
#define debug_check_raw_values_if(c,v) \
   if ((GDRAW_CORRUPTION_CHECK & ~GDRAW_CORRUPTION_MASK) == ((v) & ~GDRAW_CORRUPTION_MASK)) \
      debug_check_raw_values(c);                                                            \
   else

static void debug_set_raw_value(GDrawHandle *t)
{
   if (t->raw_ptr) {
      int i;
      for (i=0; i < 4; ++i)
         t->cached_raw_value[i] = *debug_raw_address(t, i);
      t->has_check_value = true;
   }
}

static void debug_unset_raw_value(GDrawHandle *t)
{
   t->has_check_value = false;
}

static void debug_check_value_is_unreferenced(GDrawHandleCache *c, void *ptr)
{
   GDrawHandle *t = c->head;
   while (t) {
      assert(t->raw_ptr != ptr);
      t = t->next;
   }
   t = c->active;
   while (t) {
      assert(t->raw_ptr != ptr);
      t = t->next;
   }
}

#else

#define debug_check_overlap(c,p,len)
#define debug_set_raw_value(t)
#define debug_check_value_is_unreferenced(c,p)
#define debug_unset_raw_value(t)
#define debug_check_raw_values(c)
#define debug_check_raw_values_if(c,v)
#endif

#ifdef SUPERDEBUG
static void check_lists(GDrawHandleCache *c)
{
   GDrawHandle *sentinel, *t;
   U32 state;

   // for all lists, verify that they are consistent and
   // properly linked
   for (state = 0; state < GDRAW_HANDLE_STATE__count; state++) {
      S32 count = 0;
      sentinel = &c->state[state];

      assert(!sentinel->cache);
      assert(sentinel->state == GDRAW_HANDLE_STATE_sentinel);
      for (t = sentinel->next; t != sentinel; t = t->next) {
         count++;
         assert(t->cache == c);
         assert(t->state == state);
         assert(t->prev->next == t);
         assert(t->next->prev == t);
         assert(count < 50000);
      }
   }

   // for dead list, additionally verify that it's in the right
   // order (namely, sorted by ascending fence index)
   sentinel = &c->state[GDRAW_HANDLE_STATE_dead];
   for (t = sentinel->next; t != sentinel; t = t->next) {
      assert(t->prev == sentinel || t->fence.value >= t->prev->fence.value);
   }
}

#include <stdio.h>

static const char *gdraw_StateName(U32 state)
{
   switch (state) {
   case GDRAW_HANDLE_STATE_free:       return "free";
   case GDRAW_HANDLE_STATE_live:       return "live";
   case GDRAW_HANDLE_STATE_locked:     return "locked";
   case GDRAW_HANDLE_STATE_dead:       return "dead";
   case GDRAW_HANDLE_STATE_pinned:     return "pinned";
   case GDRAW_HANDLE_STATE_user_owned: return "user-owned";
   case GDRAW_HANDLE_STATE_alloc:      return "alloc";
   case GDRAW_HANDLE_STATE_sentinel:   return "<sentinel>";
   default:                            return "???";
   }
}

#else
static RADINLINE void check_lists(GDrawHandleCache *c)
{
   RR_UNUSED_VARIABLE(c);
}
#endif

static void gdraw_HandleTransitionInsertBefore(GDrawHandle *t, GDrawHandleState new_state, GDrawHandle *succ)
{
   check_lists(t->cache);
   assert(t->state != GDRAW_HANDLE_STATE_sentinel); // sentinels should never get here!
   assert(t->state != static_cast<U32>(new_state)); // code should never call "transition" if it's not transitioning!
   // unlink from prev state
   t->prev->next = t->next;
   t->next->prev = t->prev;
   // add to list for new state
   t->next = succ;
   t->prev = succ->prev;
   t->prev->next = t;
   t->next->prev = t;
#ifdef SUPERDEBUG
   printf("GD %chandle %p %s->%s\n", t->cache->is_vertex ? 'v' : 't', t, gdraw_StateName(t->state), gdraw_StateName(new_state));
#endif
   t->state = new_state;
   check_lists(t->cache);
}

static RADINLINE void gdraw_HandleTransitionTo(GDrawHandle *t, GDrawHandleState new_state)
{
   gdraw_HandleTransitionInsertBefore(t, new_state, &t->cache->state[new_state]);
}

#ifdef GDRAW_MANAGE_MEM_TWOPOOL
static rrbool gdraw_MigrateResource(GDrawHandle *t, GDrawStats *stats);
static void gdraw_res_free(GDrawHandle *t, GDrawStats *stats);
#endif

static rrbool gdraw_HandleCacheLockStats(GDrawHandle *t, void *owner, GDrawStats *stats)
{
   RR_UNUSED_VARIABLE(stats);

   // if the GPU memory is owned by the user, then we never spontaneously
   // free it, and we can always report true. moreover, Iggy doesn't bother
   // keeping 'owner' consistent in this case, so we must check this before
   // verifying t->owner.
   if (t->state == GDRAW_HANDLE_STATE_user_owned)
      return true;

   // if t->owner has changed, then Iggy is trying to lock an old version
   // of this handle from before (the handle has already been recycled to
   // point to a new resource)
   if (t->owner != owner)
      return false;

   // otherwise, it's a valid resource and we should lock it until the next
   // unlock call
   assert(t->state == GDRAW_HANDLE_STATE_live || t->state == GDRAW_HANDLE_STATE_locked || t->state == GDRAW_HANDLE_STATE_pinned);
   if (t->state == GDRAW_HANDLE_STATE_live) {
#ifdef GDRAW_MANAGE_MEM_TWOPOOL
      // if we defragmented this frame, we can't just make resources live;
      // we need to migrate them to their new location. (which might fail
      // if we don't have enough memory left in the new pool)
      if (t->cache->did_defragment) {
         if (!gdraw_MigrateResource(t, stats)) {
            gdraw_res_free(t, stats);
            return false;
         }
      }
#endif
      gdraw_HandleTransitionTo(t, GDRAW_HANDLE_STATE_locked);
   }
   return true;
}

static rrbool gdraw_HandleCacheLock(GDrawHandle *t, void *owner)
{
   return gdraw_HandleCacheLockStats(t, owner, nullptr);
}

static void gdraw_HandleCacheUnlock(GDrawHandle *t)
{
   assert(t->state == GDRAW_HANDLE_STATE_locked || t->state == GDRAW_HANDLE_STATE_pinned || t->state == GDRAW_HANDLE_STATE_user_owned);
   if (t->state == GDRAW_HANDLE_STATE_locked)
      gdraw_HandleTransitionTo(t, GDRAW_HANDLE_STATE_live);
}

static void gdraw_HandleCacheUnlockAll(GDrawHandleCache *c)
{
   GDrawHandle *sentinel = &c->state[GDRAW_HANDLE_STATE_locked];
   while (sentinel->next != sentinel)
      gdraw_HandleTransitionTo(sentinel->next, GDRAW_HANDLE_STATE_live);
}

static void gdraw_HandleCacheInit(GDrawHandleCache *c, S32 num_handles, S32 bytes)
{
   S32 i;
   assert(num_handles > 0);
   c->max_handles = num_handles;
   c->total_bytes = bytes;
   c->bytes_free = c->total_bytes;
   c->is_vertex = false;
   c->is_thrashing = false;
   c->did_defragment = false;
   for (i=0; i < GDRAW_HANDLE_STATE__count; i++) {
      c->state[i].owner = nullptr;
      c->state[i].cache = nullptr; // should never follow cache link from sentinels!
      c->state[i].next = c->state[i].prev = &c->state[i];
#ifdef GDRAW_MANAGE_MEM
      c->state[i].raw_ptr = nullptr;
#endif
      c->state[i].fence.value = 0;
      c->state[i].bytes = 0;
      c->state[i].state = GDRAW_HANDLE_STATE_sentinel;
   }
   for (i=0; i < num_handles; ++i) {
      c->handle[i].cache  = c;
      c->handle[i].prev = (i == 0) ? &c->state[GDRAW_HANDLE_STATE_free] : &c->handle[i-1];
      c->handle[i].next = (i == num_handles - 1) ? &c->state[GDRAW_HANDLE_STATE_free] : &c->handle[i+1];
      c->handle[i].bytes = 0;
      c->handle[i].state = GDRAW_HANDLE_STATE_free;
#ifdef GDRAW_MANAGE_MEM
      c->handle[i].raw_ptr = nullptr;
#endif
   }
   c->state[GDRAW_HANDLE_STATE_free].next = &c->handle[0];
   c->state[GDRAW_HANDLE_STATE_free].prev = &c->handle[num_handles - 1];
   c->prev_frame_start.value = 0;
   c->prev_frame_end.value = 0;
#ifdef GDRAW_MANAGE_MEM
   c->alloc = nullptr;
#endif
#ifdef GDRAW_MANAGE_MEM_TWOPOOL
   c->alloc_other = nullptr;
#endif
   check_lists(c);
}

static GDrawHandle *gdraw_HandleCacheAllocateBegin(GDrawHandleCache *c)
{
   GDrawHandle *free_list = &c->state[GDRAW_HANDLE_STATE_free];
   GDrawHandle *t = nullptr;
   if (free_list->next != free_list) {
      t = free_list->next;
      gdraw_HandleTransitionTo(t, GDRAW_HANDLE_STATE_alloc);
      t->bytes = 0;
      t->owner = 0;
#ifdef GDRAW_MANAGE_MEM
      t->raw_ptr = nullptr;
#endif
#ifdef GDRAW_CORRUPTION_CHECK
      t->has_check_value = false;
#endif
   }
   return t;
}

static void gdraw_HandleCacheAllocateEnd(GDrawHandle *t, S32 bytes, void *owner, GDrawHandleState new_state)
{
   assert(t->cache);
   assert(t->bytes == 0);
   assert(t->owner == 0);
   assert(t->state == GDRAW_HANDLE_STATE_alloc);
   // 4J Stu - Need to keep the braces here for PS3 compiler and our version of assert
   if (bytes == 0)
   {
      assert(new_state == GDRAW_HANDLE_STATE_user_owned);
   }
   else
   {
      assert(new_state == GDRAW_HANDLE_STATE_locked || new_state == GDRAW_HANDLE_STATE_pinned);
   }
   t->bytes = bytes;
   t->owner = owner;
   t->cache->bytes_free -= bytes;

   gdraw_HandleTransitionTo(t, new_state);
}

static void gdraw_HandleCacheFree(GDrawHandle *t)
{
   GDrawHandleCache *c = t->cache;
   assert(t->state != GDRAW_HANDLE_STATE_alloc && t->state != GDRAW_HANDLE_STATE_sentinel);
   c->bytes_free += t->bytes;
   t->bytes = 0;
   t->owner = 0;
#ifdef GDRAW_MANAGE_MEM
   t->raw_ptr = 0;
#endif
#ifdef GDRAW_CORRUPTION_CHECK
   t->has_check_value = false;
#endif
   gdraw_HandleTransitionTo(t, GDRAW_HANDLE_STATE_free);
}

static void gdraw_HandleCacheAllocateFail(GDrawHandle *t)
{
   assert(t->state == GDRAW_HANDLE_STATE_alloc);
   gdraw_HandleTransitionTo(t, GDRAW_HANDLE_STATE_free);
}

static GDrawHandle *gdraw_HandleCacheGetLRU(GDrawHandleCache *c)
{
   // TransitionTo always inserts at the end, which means that the resources
   // at the front of the LRU list are the oldest ones, since in-use resources
   // will get appended on every transition from "locked" to "live".
   GDrawHandle *sentinel = &c->state[GDRAW_HANDLE_STATE_live];
   return (sentinel->next != sentinel) ? sentinel->next : nullptr;
}

static void gdraw_HandleCacheTick(GDrawHandleCache *c, GDrawFence now)
{
   c->prev_frame_start = c->prev_frame_end;
   c->prev_frame_end = now;

   // reset these flags every frame
   c->is_thrashing = false;
   c->did_defragment = false;
}

#ifdef GDRAW_MANAGE_MEM

static void gdraw_HandleCacheInsertDead(GDrawHandle *t)
{
   GDrawHandle *s, *sentinel;

   assert(t->state == GDRAW_HANDLE_STATE_live || t->state == GDRAW_HANDLE_STATE_locked || t->state == GDRAW_HANDLE_STATE_pinned);

   // figure out where t belongs in the dead list in "chronological order"
   // do this by finding its (chronological) successor s
   sentinel = &t->cache->state[GDRAW_HANDLE_STATE_dead];
   s = sentinel->next;
   while (s != sentinel && s->fence.value <= t->fence.value)
      s = s->next;

   // and then insert it there
   gdraw_HandleTransitionInsertBefore(t, GDRAW_HANDLE_STATE_dead, s);
}

#endif

////////////////////////////////////////////////////////////////////////
//
//   Set transformation matrices
//

// Our vertex shaders use this convention:
//   world: our world matrices always look like this
//       m00  m01  0  t0
//       m10  m11  0  t1
//        0    0   0   d
//        0    0   0   1
//
//      we just store the first two rows and insert d
//      in the first row, third column. our input position vectors are
//      always (x,y,0,1) or (x,y,0,0), so we can still just use dp4 to
//      compute final x/y. after that it's a single move to set the
//      correct depth value.
//
//   viewproj: our view-projection matrix is always just a 2D scale+translate,
//     i.e. the matrix looks like this:
//
//       p[0]  0    0   p[2]
//         0  p[1]  0   p[3]
//         0   0    1    0
//         0   0    0    1
//
//     just store (p[0],p[1],p[2],p[3]) in a 4-component vector and the projection
//     transform is a single multiply-add.
//
// The output is volatile since it's often in Write-Combined memory where we
// really don't want compiler reordering.

static RADINLINE void gdraw_PixelSpace(volatile F32 * RADRESTRICT vvec)
{
   // 1:1 pixel mapping - just identity since our "view space" is pixels
   vvec[0] = 1.0f;   vvec[1] = 0.0f;   vvec[2] = 0.0f;   vvec[3] = 0.0f;
   vvec[4] = 0.0f;   vvec[5] = 1.0f;   vvec[6] = 0.0f;   vvec[7] = 0.0f;
}

static RADINLINE void gdraw_WorldSpace(volatile F32 * RADRESTRICT vvec, F32 * RADRESTRICT world_to_pixel, F32 depth, F32 misc)
{
   // World->pixel space transform is just a scale
   vvec[0] = world_to_pixel[0];  vvec[1] = 0.0f;               vvec[2] = depth;  vvec[3] = 0.0f;
   vvec[4] = 0.0f;               vvec[5] = world_to_pixel[1];  vvec[6] = misc;   vvec[7] = 0.0f;
}

static RADINLINE void gdraw_ObjectSpace(volatile F32 * RADRESTRICT vvec, gswf_matrix * RADRESTRICT xform, F32 depth, F32 misc)
{
   // Object->pixel transform is a 2D homogeneous matrix transform
   F32 m00 = xform->m00;
   F32 m01 = xform->m01;
   F32 m10 = xform->m10;
   F32 m11 = xform->m11;
   F32 trans0 = xform->trans[0];
   F32 trans1 = xform->trans[1];

   vvec[0] = m00; vvec[1] = m01; vvec[2] = depth;  vvec[3] = trans0;
   vvec[4] = m10; vvec[5] = m11; vvec[6] = misc;   vvec[7] = trans1;
}

static void gdraw_GetObjectSpaceMatrix(F32 * RADRESTRICT mat, gswf_matrix * RADRESTRICT xform, F32 * RADRESTRICT proj, F32 depth, int out_col_major)
{
   int row = out_col_major ? 1 : 4;
   int col = out_col_major ? 4 : 1;

   F32 xs = proj[0];
   F32 ys = proj[1];

   mat[0*row+0*col] = xform->m00      * xs;
   mat[0*row+1*col] = xform->m01      * xs;
   mat[0*row+2*col] = 0.0f;
   mat[0*row+3*col] = xform->trans[0] * xs + proj[2];

   mat[1*row+0*col] = xform->m10      * ys;
   mat[1*row+1*col] = xform->m11      * ys;
   mat[1*row+2*col] = 0.0f;
   mat[1*row+3*col] = xform->trans[1] * ys + proj[3];

   mat[2*row+0*col] = 0.0f;
   mat[2*row+1*col] = 0.0f;
   mat[2*row+2*col] = 0.0f;
   mat[2*row+3*col] = depth;

   mat[3*row+0*col] = 0.0f;
   mat[3*row+1*col] = 0.0f;
   mat[3*row+2*col] = 0.0f;
   mat[3*row+3*col] = 1.0f;
}


////////////////////////////////////////////////////////////////////////
//
//   Blurs
//
// symmetrically expand a rectangle by ex/ey pixels on both sides, then clamp to tile bounds
static void gdraw_ExpandRect(gswf_recti *out, gswf_recti const *in, S32 ex, S32 ey, S32 w, S32 h)
{
   out->x0 = RR_MAX(in->x0 - ex, 0);
   out->y0 = RR_MAX(in->y0 - ey, 0);
   out->x1 = RR_MIN(in->x1 + ex, w);
   out->y1 = RR_MIN(in->y1 + ey, h);
}

static void gdraw_ShiftRect(gswf_recti *out, gswf_recti const *in, S32 dx, S32 dy)
{
   out->x0 = in->x0 + dx;
   out->y0 = in->y0 + dy;
   out->x1 = in->x1 + dx;
   out->y1 = in->y1 + dy;
}

#define MAX_TAPS  9  // max # of bilinear samples in one 'convolution' step

enum
{
   // basic shader family
   VAR_tex0 = 0,
   VAR_tex1,
   VAR_cmul,
   VAR_cadd,
   VAR_focal,

   // filter family
   VAR_filter_tex0  = 0,
   VAR_filter_tex1,
   VAR_filter_color,
   VAR_filter_tc_off,
   VAR_filter_tex2,
   VAR_filter_clamp0,
   VAR_filter_clamp1,
   VAR_filter_color2,
   MAX_VARS,

   // blur family
   VAR_blur_tex0 = 0,
   VAR_blur_tap,
   VAR_blur_clampv,

   // color matrix family
   VAR_colormatrix_tex0 = 0,
   VAR_colormatrix_data,

   // ihud family
   VAR_ihudv_worldview = 0,
   VAR_ihudv_material,
   VAR_ihudv_textmode,
};

typedef struct
{
   S32 w,h, frametex_width, frametex_height;
   void (*BlurPass)(GDrawRenderState *r, int taps,  float *data, gswf_recti *s, float *tc, float height_max, float *clampv, GDrawStats *gstats);
} GDrawBlurInfo;

static GDrawTexture *gdraw_BlurPass(GDrawFunctions *g, GDrawBlurInfo *c, GDrawRenderState *r, int taps, float *data, gswf_recti *draw_bounds, gswf_recti *sample_bounds, GDrawStats *gstats)
{
   F32 tc[4];
   F32 clamp[4];
   F32 t=0;
   F32 texel_scale_s = 1.0f / c->frametex_width;
   F32 texel_scale_t = 1.0f / c->frametex_height;
   S32 i;
   for (i=0; i < taps; ++i)
      t += data[4*i+2];
   assert(t >= 0.99f && t <= 1.01f);

   tc[0] = texel_scale_s * draw_bounds->x0;
   tc[1] = texel_scale_t * draw_bounds->y0;
   tc[2] = texel_scale_s * draw_bounds->x1;
   tc[3] = texel_scale_t * draw_bounds->y1;

   // sample_bounds is (x0,y0) inclusive, (x1,y1) exclusive
   // texel centers are offset by 0.5 from integer coordinates and we don't want to sample outside sample_bounds
   clamp[0] = texel_scale_s * (sample_bounds->x0 + 0.5f);
   clamp[1] = texel_scale_t * (sample_bounds->y0 + 0.5f);
   clamp[2] = texel_scale_s * (sample_bounds->x1 - 0.5f);
   clamp[3] = texel_scale_t * (sample_bounds->y1 - 0.5f);

   if (!g->TextureDrawBufferBegin(draw_bounds, GDRAW_TEXTURE_FORMAT_rgba32, GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_color | GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_alpha, 0, gstats))
      return r->tex[0];

   c->BlurPass(r, taps, data, draw_bounds, tc, static_cast<F32>(c->h) / c->frametex_height, clamp, gstats);
   return g->TextureDrawBufferEnd(gstats);
}

static GDrawTexture *gdraw_BlurPassDownsample(GDrawFunctions *g, GDrawBlurInfo *c, GDrawRenderState *r, int taps, float *data, gswf_recti *draw_bounds, int axis, int divisor, int tex_w, int tex_h, gswf_recti *sample_bounds, GDrawStats *gstats)
{
   S32 i;
   F32 t=0;
   F32 tc[4];
   F32 clamp[4];
   F32 texel_scale_s = 1.0f / tex_w;
   F32 texel_scale_t = 1.0f / tex_h;
   gswf_recti z;

   for (i=0; i < taps; ++i)
      t += data[4*i+2];
   assert(t >= 0.99f && t <= 1.01f);

   // following must be integer divides!
   if (axis == 0) {
      z.x0 = draw_bounds->x0 / divisor;
      z.x1 = (draw_bounds->x1-1) / divisor + 1;
      z.y0 = draw_bounds->y0;
      z.y1 = draw_bounds->y1;

      tc[0] = ((z.x0 - 0.5f)*divisor+0.5f)*texel_scale_s;
      tc[2] = ((z.x1 - 0.5f)*divisor+0.5f)*texel_scale_s;
      tc[1] = z.y0*texel_scale_t;
      tc[3] = z.y1*texel_scale_t;
   } else {
      z.x0 = draw_bounds->x0;
      z.x1 = draw_bounds->x1;
      z.y0 = draw_bounds->y0 / divisor;
      z.y1 = (draw_bounds->y1-1) / divisor + 1;

      tc[0] = z.x0*texel_scale_s;
      tc[2] = z.x1*texel_scale_s;
      tc[1] = ((z.y0 - 0.5f)*divisor+0.5f)*texel_scale_t;
      tc[3] = ((z.y1 - 0.5f)*divisor+0.5f)*texel_scale_t;
   }

   if (!g->TextureDrawBufferBegin(&z, GDRAW_TEXTURE_FORMAT_rgba32, GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_color | GDRAW_TEXTUREDRAWBUFFER_FLAGS_needs_alpha, 0, gstats))
      return r->tex[0];

   clamp[0] = texel_scale_s * (sample_bounds->x0 + 0.5f);
   clamp[1] = texel_scale_t * (sample_bounds->y0 + 0.5f);
   clamp[2] = texel_scale_s * (sample_bounds->x1 - 0.5f);
   clamp[3] = texel_scale_t * (sample_bounds->y1 - 0.5f);

   assert(clamp[0] <= clamp[2]);
   assert(clamp[1] <= clamp[3]);

   c->BlurPass(r, taps, data, &z, tc, static_cast<F32>(c->h) / c->frametex_height, clamp, gstats);
   return g->TextureDrawBufferEnd(gstats);
}

#define unmap(t,a,b)             (((t)-(a))/(F32) ((b)-(a)))
#define linear_remap(t,a,b,c,d)  ((c) + unmap(t,a,b)*((d)-(c)))

static void gdraw_BlurAxis(S32 axis, GDrawFunctions *g, GDrawBlurInfo *c, GDrawRenderState *r, F32 blur_width, F32 texel, gswf_recti *draw_bounds, gswf_recti *sample_bounds, GDrawTexture *protect, GDrawStats *gstats)
{
   GDrawTexture *t;
   F32 data[MAX_TAPS][4];
   S32 off_axis = 1-axis;
   S32 w = static_cast<S32>(ceil((blur_width - 1) / 2))*2+1;  // 1.2 => 3, 2.8 => 3, 3.2 => 5
   F32 edge_weight = 1 - (w - blur_width)/2;   // 3 => 0 => 1;   1.2 => 1.8 => 0.9 => 0.1
   F32 inverse_weight = 1.0f / blur_width;

   w = ((w-1) >> 1) + 1; // 3 => 2, 5 => 3, 7 => 4   (number of texture samples)

   if (!r->tex[0])
      return;

   // horizontal filter
   if (w > 1) {
      if (w <= MAX_TAPS) {
         // we have enough taps to just do it
         // use 'w' taps
         S32 i, expand;

         // just go through and place all the taps in the right place

         // if w is 2 (sample from -1,0,1)
         //    0 => -0.5
         //    1 =>  1

         // if w is 3:
         //    0 => -1.5   samples from -2,-1
         //    1 =>  0.5   samples from 0,1
         //    2 =>  2     samples from 2

         // if w is 4:
         //    0 => -2.5   samples from -3,-2
         //    1 => -0.5   samples from -1,0
         //    2 =>  1.5   samples from  1,2
         //    3 =>  3     samples from  3

         for (i=0; i < w; ++i) {
            // first texsample samples from -w+1 and -w+2, e.g. w=2 => -1,0,1
            data[i][axis] = (-w+1.5f + i*2)*texel;
            data[i][off_axis] = 0;
            data[i][2] = 2*inverse_weight; // 2 full-weight samples
            data[i][3] = 0;
         }
         // now reweight the last one
         data[i-1][axis] = (w-1)*texel;
         data[i-1][2] = edge_weight*inverse_weight;
         // now reweight the first one
         //     (ew*0 + 1*1)/(1+ew) = 1/(1+ew)
         data[0][axis] = (-w + 1.0f + 1/(edge_weight+1)) * texel;
         data[0][2] = (edge_weight+1)*inverse_weight;

         expand = w-1;
         gdraw_ExpandRect(draw_bounds, draw_bounds, axis ? 0 : expand, axis ? expand : 0, c->w, c->h);

         t = gdraw_BlurPass(g, c, r, w, data[0], draw_bounds, sample_bounds, gstats);
         if (r->tex[0] != protect && r->tex[0] != t)
            g->FreeTexture(r->tex[0], 0, gstats);
         r->tex[0] = t;
         gdraw_ExpandRect(sample_bounds, draw_bounds, 1, 1, c->w, c->h); // for next pass
      } else {
         // @OPTIMIZE: for symmetrical blurs we can get a 2-wide blur in the *off* axis at the same
         // time we get N-wide in the on axis, which could double our max width
         S32 i, expand;
         // @HACK: this is really a dumb way to do it, i kind of had a brain fart, you could get
         // the exact same result by just doing the downsample the naive way and then the
         // final sample uses texture samples spaced by a texel rather than spaced by two
         // texels -- the current method is just as inefficient, it just puts the inefficiency
         // in the way the downsampled texture is self-overlapping, so the downsampled texture
         // is twice as larger as it should be.

         // we COULD be exact by generating a mipmap, then sampling some number of samples
         // from the mipmap and some from the original, but that would require being polyphase.
         // instead we just are approximate. the mipmap weights the edge pixels by one half
         // and overlaps them by one sample, so then in phase two we sample N slightly-overlapping
         // mipmap samples
         //
         // instead we do the following.
         //    divide the source data up into clusters that are K samples long.
         //            ...K0...   ...K1...  ...K2...   ...K3...
         //
         // Suppose K[i] is the average of all the items in cluster i.
         //
         // We compute a downsampled texture where T[i] = K[i] + K[i+1].
         //
         // Now, we sample N taps from adjacent elements of T, allowing the texture unit
         // to bilerp. Suppose a given sample falls at coordinate i with sub-position p.
         // Then tap #j will compute:
         //      T[i+j]*(1-p) + T[i+j+1]*p
         // But tap #j+1 will compute:
         //      T[i+j+1]*(1-p) + T[i+j+2]*p
         // so we end up computing:
         //      sum(T[i+j]) except for the end samples.
         //
         // So, how do we create these initial clusters? That's easy, we use K taps
         // to sample 2K texels.
         //
         // What value of k do we use? Well, we're constrained to using MAX_TAPS
         // on each pass. So at the high end, we're bounded by:
         //      K = MAX_TAPS
         //      S = MAX_TAPS (S is number of samples in second pass)
         // S addresses S*2-1 texels of T, and each texel adds K more samples,
         // so (ignoring the edges) we basically have w = K*S

         // if w == MAX_TAPS*MAX_TAPS, then k = MAX_TAPS
         // if w == MAX_TAPS+1, then k = 2
         //
         // suppose we have 3 taps, then we can sample 5 samples in one pass, so then our
         // max coverage is 25 samples, or a filter width of 13. with 7 taps, we sample
         // 13 samples in one pass, max coverage is 13*13 samples or (13*13-1)/2 width,
         // which is ((2T-1)*(2T-1)-1)/2 or (4T^2 - 4T + 1 -1)/2 or 2T^2 - 2T or 2T*(T-1)
         S32 w_mip = static_cast<S32>(ceil(linear_remap(w, MAX_TAPS+1, MAX_TAPS*MAX_TAPS, 2, MAX_TAPS)));
         S32 downsample = w_mip;
         F32 sample_spacing = texel;
         if (downsample < 2) downsample = 2;
         if (w_mip > MAX_TAPS) {
            // if w_mip > MAX_TAPS, then we ought to use more than one mipmap pass, but
            // since that's a huge filter ( > 80 pixels) let's just try subsampling and
            // see if it's good enough.
            sample_spacing *= w_mip / MAX_TAPS;
            w_mip = MAX_TAPS;
         } else {
            assert(w / downsample <= MAX_TAPS);
         }
         inverse_weight = 1.0f / (2*w_mip);
         for (i=0; i < w_mip; ++i) {
            data[i][axis] = (-w_mip+1 + i*2+0.5f)*sample_spacing;
            data[i][off_axis] = 0;
            data[i][2] = 2*inverse_weight;
            data[i][3] = 0;
         }
         w = w*2 / w_mip;

         // @TODO: compute the correct bboxes for this size
         // the downsampled texture samples from -w_mip+1 to w_mip
         // the sample from within that samples w spots within that,
         // or w/2 of those, but they're overlapping by 50%.
         // so if a sample is a point i, it samples from the original
         // from -w_mip+1 to w_mip + i*w_mip.
         // So then the minimum is: -w_mip+1 + (w/2)*w_mip, and
         // the maximum is w_mip + (w/2)*w_mip
         expand = (((w+1)>>1)+1)*w_mip+1;
         gdraw_ExpandRect(draw_bounds, draw_bounds, axis ? 0 : expand, axis ? expand : 0, c->w, c->h);
         
         t = gdraw_BlurPassDownsample(g, c, r, w_mip, data[0], draw_bounds, axis, downsample, c->frametex_width, c->frametex_height, sample_bounds, gstats);
         if (r->tex[0] != protect && r->tex[0] != t)
            g->FreeTexture(r->tex[0], 0, gstats);
         r->tex[0] = t;
         gdraw_ExpandRect(sample_bounds, draw_bounds, 1, 1, c->w, c->h);
         if (!r->tex[0])
            return;

         // now do a regular blur pass sampling from that
         // the raw texture now contains 'downsample' samples per texel
         if (w > 2*MAX_TAPS) {
            sample_spacing = texel * (w-1) / (2*MAX_TAPS-1);
            w = 2*MAX_TAPS;
         } else {
            sample_spacing = texel;
         }
         //sample_spacing *= 1.0f/2;
         assert(w >= 2 && w <= 2*MAX_TAPS);

         if (w & 1) {
            // we just want to evenly weight even-spaced samples
            inverse_weight = 1.0f / w;

            // just go through and place all the taps in the right place

            w = (w+1)>>1;
            for (i=0; i < w; ++i) {
               data[i][axis] = (-w+1.0f + 0.5f + i*2)*sample_spacing;
               data[i][off_axis] = 0;
               data[i][2] = 2*inverse_weight; // 2 full-weight samples
               data[i][3] = 0;
            }

            // fix up the last tap

            // the following test is always true, but we're testing it here
            // explicitly so as to make VS2012's static analyzer not complain
            if (i > 0) {
               data[i-1][axis] = (-w+1.0f+(i-1)*2)*sample_spacing;
               data[i-1][2] = inverse_weight;
            }
         } else {
            // we just want to evenly weight even-spaced samples
            inverse_weight = 1.0f / w;

            // just go through and place all the taps in the right place
            w >>= 1;
            for (i=0; i < w; ++i) {
               data[i][axis] = (-w+1.0f + i*2)*sample_spacing;
               data[i][off_axis] = 0;
               data[i][2] = 2*inverse_weight; // 2 full-weight samples
               data[i][3] = 0;
            }
         }

         t = gdraw_BlurPassDownsample(g, c, r, w, data[0], draw_bounds, axis, 1,
                axis==0 ? c->frametex_width*downsample : c->frametex_width,
                axis==1 ? c->frametex_height*downsample : c->frametex_height, sample_bounds, gstats);
         if (r->tex[0] != protect && r->tex[0] != t)
            g->FreeTexture(r->tex[0], 0, gstats);
         r->tex[0] = t;
         gdraw_ExpandRect(sample_bounds, draw_bounds, 1, 1, c->w, c->h);
      }
   }
}

static void gdraw_Blur(GDrawFunctions *g, GDrawBlurInfo *c, GDrawRenderState *r, gswf_recti *draw_bounds, gswf_recti *sample_bounds, GDrawStats *gstats)
{
   S32 p;
   GDrawTexture *protect = r->tex[0];
   gswf_recti sbounds;

   // compute texel offset size
   F32 dx = 1.0f / c->frametex_width;
   F32 dy = 1.0f / c->frametex_height;

   // blur = 1 => 1 tap
   // blur = 1.2 => 3 taps (0.1, 1, 0.1)
   // blur = 2.2 => 3 taps (0.6, 1, 0.6)
   // blur = 2.8 => 3 taps (0.9, 1, 0.9)
   // blur = 3   => 3 taps (1  , 1, 1  )
   // blur = 3.2 => 5 taps (0.1, 1, 1, 1, 0.1)

   //S32 w = ((S32) ceil((r->blur_x-1)/2))*2+1;   // 1.2 => (1.2-1)/2 => 0.1 => 1.0 => 1 => 2 => 3
   //S32 h = ((S32) ceil((r->blur_y-1)/2))*2+1;   // 3   => (3-1)/2 => 1.0 => 1 => 2 => 3

   // gdraw puts 1 border pixel around everything when producing rendertargets and we use this
   // so expand the input sample bounds accordingly
   gdraw_ExpandRect(&sbounds, sample_bounds, 1, 1, c->w, c->h);

   for (p=0; p < r->blur_passes; ++p) {
      #if 0 // @OPTIMIZE do the filter in one pass
      if (w*h <= MAX_TAPS) {
      } else
      #endif
      {
         // do the filter separably
         gdraw_BlurAxis(0,g,c,r,r->blur_x,dx, draw_bounds, &sbounds, protect, gstats);
         gdraw_BlurAxis(1,g,c,r,r->blur_y,dy, draw_bounds, &sbounds, protect, gstats);
      }
   }
}

#ifdef GDRAW_MANAGE_MEM

static void make_pool_aligned(void **start, S32 *num_bytes, U32 alignment)
{
   UINTa addr_orig = (UINTa) *start;
   UINTa addr_aligned = (addr_orig + alignment-1) & ~((UINTa) alignment - 1);

   if (addr_aligned != addr_orig) {
      S32 diff = (S32) (addr_aligned - addr_orig);
      if (*num_bytes < diff) { 
         *start = nullptr;
         *num_bytes = 0;
         return;
      } else {
         *start = (void *)addr_aligned;
         *num_bytes -= diff;
      }
   }
}

// Very simple arena allocator
typedef struct
{
   U8 *begin;
   U8 *current;
   U8 *end;
} GDrawArena;

static void gdraw_arena_init(GDrawArena *arena, void *start, U32 size)
{
   arena->begin = (U8 *)start;
   arena->current = (U8 *)start;
   arena->end = (U8 *)start + size;
}

static GDRAW_MAYBE_UNUSED void gdraw_arena_reset(GDrawArena *arena)
{
   arena->current = arena->begin;
}

static void *gdraw_arena_alloc(GDrawArena *arena, U32 size, U32 align)
{
   UINTa start_addr = ((UINTa)arena->current + align-1) & ~((UINTa) align - 1);
   U8 *ptr = (U8 *)start_addr;
   UINTa remaining = arena->end - arena->current;
   UINTa total_size = (ptr - arena->current) + size;
   if (remaining < total_size) // doesn't fit
      return nullptr;

   arena->current = ptr + size;
   return ptr;
}

// Allocator for graphics memory.
// Graphics memory is assumed to be write-combined and slow to read for the
// CPU, so we keep all heap management information separately in main memory.
//
// There's a constant management of about 1k (2k for 64bit) to create a heap,
// plus a per-block overhead. The maximum number of blocks the allocator can
// ever use is bounded by 2*max_allocs+1; since GDraw manages a limited
// amount of handles, max_allocs is a known value at heap creation time.
//
// The allocator uses a best-fit heuristic to minimize fragmentation.
// Currently, there are no size classes or other auxiliary data structures to
// speed up this process, since the number of free blocks at any point in time
// is assumed to be fairly low.
//
// The allocator maintains a number of invariants:
// - The free list and physical block list are proper double-linked lists.
//   (i.e. block->next->prev == block->prev->next == block)
// - All allocated blocks are also kept in a hash table, indexed by their
//   pointer (to allow free to locate the corresponding block_info quickly).
//   There's a single-linked, nullptr-terminated list of elements in each hash
//   bucket.
// - The physical block list is ordered. It always contains all currently
//   active blocks and spans the whole managed memory range. There are no
//   gaps between blocks, and all blocks have nonzero size.
// - There are no two adjacent free blocks; if two such blocks would be created,
//   they are coalesced immediately.
// - The maximum number of blocks that could ever be necessary is allocated
//   on initialization. All block_infos not currently in use are kept in a
//   single-linked, nullptr-terminated list of unused blocks. Every block is either
//   in the physical block list or the unused list, and the total number of
//   blocks is constant.
// These invariants always hold before and after an allocation/free.

#ifndef GFXALLOC_ASSERT
#define GFXALLOC_ASSERT(x)
#endif

typedef struct gfx_block_info
{
   U8 *ptr;
   gfx_block_info *prev, *next; // for free blocks this is the free list, for allocated blocks it's a (single-linked!) list of elements in the corresponding hash bucket
   gfx_block_info *prev_phys, *next_phys;
   U32 is_free : 1;
   U32 is_unused : 1;
   U32 size : 30;
} gfx_block_info;
// 24 bytes/block on 32bit, 48 bytes/block on 64bit.

#define GFXALLOC_HASH_SIZE 256

typedef struct gfx_allocator
{
   U8 *mem_base;
   U8 *mem_end;
   U32 max_allocs;
   U32 block_align;
   U32 block_shift;
   S32 actual_bytes_free;

#ifdef GFXALLOC_CHECK
   int num_blocks;
   int num_unused;
   int num_alloc;
   int num_free;
#endif

   GDrawHandleCache *cache;

   gfx_block_info *unused_list; // next unused block_info (single-linked list)
   gfx_block_info *hash[GFXALLOC_HASH_SIZE]; // allocated blocks
   gfx_block_info blocks[1]; // first block is head of free list AND head of physical block list (sentinel)
} gfx_allocator;
// about 1k (32bit), 2k (64bit) with 256 hash buckets (the default). dominated by hash table.

#ifdef GFXALLOC_CHECK
#define GFXALLOC_IF_CHECK(x)  x
#else
#define GFXALLOC_IF_CHECK(x)
#endif

static U32 gfxalloc_get_hash_code(gfx_allocator *alloc, void *ptr)
{
   U32 a = (U32) (((U8 *) ptr - alloc->mem_base) >> alloc->block_shift);

   // integer hash function by Bob Jenkins (http://burtleburtle.net/bob/hash/integer.html)
   // I use this function because integer mults are slow on PPC and large literal constants
   // take multiple instrs to set up on all RISC CPUs.
   a -= (a<<6);
   a ^= (a>>17);
   a -= (a<<9);
   a ^= (a<<4);
   a -= (a<<3);
   a ^= (a<<10);
   a ^= (a>>15);

   return a & (GFXALLOC_HASH_SIZE - 1);
}

#if defined(SUPERDEBUG) || defined(COMPLETE_DEBUG)
#include <stdlib.h>
#define MAX_REGIONS  8192
typedef struct
{
   U32 begin,end;
} gfx_region;
static gfx_region region[MAX_REGIONS];

static int region_sort(const void *p, const void *q)
{
   U32 a = *(U32*)p;
   U32 b = *(U32*)q;
   if (a < b) return -1;
   if (a > b) return  1;
   return 0;
}

static void gfxalloc_check1(gfx_allocator *alloc)
{
   assert(alloc->max_allocs*2+1 < MAX_REGIONS);
   int i,n=0;
   for (i=0; i < GFXALLOC_HASH_SIZE; ++i) {
      gfx_block_info *b = alloc->hash[i];
      while (b) {
         region[n].begin = (UINTa) b->ptr;
         region[n].end = region[n].begin + b->size;
         ++n;
         b = b->next;
      }
   }
   gfx_block_info *b = alloc->blocks[0].next;
   while (b != &alloc->blocks[0]) {
      region[n].begin = (UINTa) b->ptr;
      region[n].end = region[n].begin + b->size;
      ++n;
      b = b->next;
   }
   qsort(region, n, sizeof(region[0]), region_sort);
   for (i=0; i+1 < n; ++i) {
      assert(region[i].end == region[i+1].begin);
   }
}
#else
#define gfxalloc_check1(a)
#endif

#ifdef COMPLETE_DEBUG
static void verify_against_blocks(int num_regions, void *vptr, S32 len)
{
   U32 *ptr = (U32 *) vptr;
   // binary search for ptr amongst regions
   S32 s=0,e=num_regions-1;
   assert(len != 0);
   while (s < e) {
      S32 i = (s+e+1)>>1;
      // invariant:  b[s] <= ptr <= b[e]
      if (region[i].begin <= (UINTa) ptr)
         s = i;
      else
         e = i-1;

      // consider cases:
      //    s=0,e=1: i = 0, how do we get i to be 1?
   }
   // at this point, s >= e
   assert(s < num_regions && region[s].begin == (UINTa) ptr && (UINTa) ptr+len <= region[s].end);
}

static void debug_complete_check(gfx_allocator *alloc, void *ptr, S32 len, void *skip)
{
   GDrawHandleCache *c = alloc->cache;
   assert(alloc->max_allocs*2+1 < MAX_REGIONS);
   int i,n=0;
   for (i=0; i < GFXALLOC_HASH_SIZE; ++i) {
      gfx_block_info *b = alloc->hash[i];
      while (b) {
         region[n].begin = (UINTa) b->ptr;
         region[n].end = region[n].begin + b->size;
         ++n;
         b = b->next;
      }
   }
   gfx_block_info *b = alloc->blocks[0].next;
   while (b != &alloc->blocks[0]) {
      region[n].begin = (UINTa) b->ptr;
      region[n].end = region[n].begin + b->size;
      ++n;
      b = b->next;
   }
   for (i=0; i < n; ++i)
      assert(region[i].end > region[i].begin);
   qsort(region, n, sizeof(region[0]), region_sort);
   for (i=0; i+1 < n; ++i) {
      assert(region[i].end == region[i+1].begin);
   }

   if (ptr)
      verify_against_blocks(n, ptr, len);

   if (c) {
      GDrawHandle *t = c->head;
      while (t) {
         if (t->raw_ptr && t->raw_ptr != skip)
            verify_against_blocks(n, t->raw_ptr, t->bytes);
         t = t->next;
      }
      t = c->active;
      while (t) {
         if (t->raw_ptr && t->raw_ptr != skip)
            verify_against_blocks(n, t->raw_ptr, t->bytes);
         t = t->next;
      }
   }
}
#else
#define debug_complete_check(a,p,len,s)
#endif

#ifdef GFXALLOC_CHECK
static void gfxalloc_check2(gfx_allocator *alloc)
{
   int n=0;
   gfx_block_info *b = alloc->unused_list;
   while (b) {
      ++n;
      b = b->next;
   }
   GFXALLOC_ASSERT(n == alloc->num_unused);
   b = alloc->blocks->next;
   n = 0;
   while (b != alloc->blocks) {
      ++n;
      b = b->next;
   }
   GFXALLOC_ASSERT(n == alloc->num_free);
   GFXALLOC_ASSERT(alloc->num_blocks == alloc->num_unused + alloc->num_free + alloc->num_alloc);
}
#define gfxalloc_check(a) do { gfxalloc_check1(a); gfxalloc_check2(a); } while(0)
#else
#define gfxalloc_check2(a)
#define gfxalloc_check(a)
#endif



static gfx_block_info *gfxalloc_pop_unused(gfx_allocator *alloc)
{
   GFXALLOC_ASSERT(alloc->unused_list != nullptr);
   GFXALLOC_ASSERT(alloc->unused_list->is_unused);
   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_unused);)

   gfx_block_info *b = alloc->unused_list;
   alloc->unused_list = b->next;
   GFXALLOC_ASSERT(alloc->unused_list);
   b->is_unused = 0;
   GFXALLOC_IF_CHECK(--alloc->num_unused;)
   return b;
}

static void gfxalloc_push_unused(gfx_allocator *alloc, gfx_block_info *b)
{
   GFXALLOC_ASSERT(!b->is_unused);
   b->is_unused = 1;
   b->next = alloc->unused_list;
   alloc->unused_list = b;
   GFXALLOC_IF_CHECK(++alloc->num_unused);
}

static void gfxalloc_add_free(gfx_allocator *alloc, gfx_block_info *b)
{
   gfx_block_info *head = alloc->blocks;

   b->is_free = 1;
   b->next = head->next;
   b->prev = head;
   head->next->prev = b;
   head->next = b;
   GFXALLOC_IF_CHECK(++alloc->num_free;)
}

static void gfxalloc_rem_free(gfx_allocator *alloc, gfx_block_info *b)
{
   RR_UNUSED_VARIABLE(alloc);
   b->is_free = 0;
   b->prev->next = b->next;
   b->next->prev = b->prev;
   GFXALLOC_IF_CHECK(--alloc->num_free;)
}

static void gfxalloc_split_free(gfx_allocator *alloc, gfx_block_info *b, U32 pos)
{
   gfx_block_info *n = gfxalloc_pop_unused(alloc);

   GFXALLOC_ASSERT(b->is_free);
   GFXALLOC_ASSERT(pos > 0 && pos < b->size);

   // set up new free block
   n->ptr = b->ptr + pos;
   n->prev_phys = b;
   n->next_phys = b->next_phys;
   n->next_phys->prev_phys = n;
   n->size = b->size - pos;
   assert(n->size != 0);
   gfxalloc_add_free(alloc, n);

   // fix original block
   b->next_phys = n;
   b->size = pos;
   assert(b->size != 0);

debug_complete_check(alloc, n->ptr, n->size,0);
debug_complete_check(alloc, b->ptr, b->size,0);
}

static gfx_allocator *gfxalloc_create(void *mem, U32 mem_size, U32 align, U32 max_allocs)
{
   gfx_allocator *a;
   U32 i, max_blocks, size;

   if (!align || (align & (align - 1)) != 0) // align must be >0 and a power of 2
      return nullptr;

   // for <= max_allocs live allocs, there's <= 2*max_allocs+1 blocks. worst case:
   // [free][used][free] .... [free][used][free]
   max_blocks = max_allocs * 2 + 1;
   size = sizeof(gfx_allocator) + max_blocks * sizeof(gfx_block_info);
   a = (gfx_allocator *) IggyGDrawMalloc(size);
   if (!a)
      return nullptr;

   memset(a, 0, size);

   GFXALLOC_IF_CHECK(a->num_blocks = max_blocks;)
   GFXALLOC_IF_CHECK(a->num_alloc  = 0;)
   GFXALLOC_IF_CHECK(a->num_free   = 1;)
   GFXALLOC_IF_CHECK(a->num_unused = max_blocks-1;)

   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(a->num_blocks == a->num_alloc + a->num_free + a->num_unused);)
   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(a->num_free <= a->num_blocks+1);)

   a->actual_bytes_free = mem_size;
   a->mem_base = (U8 *) mem;
   a->mem_end = a->mem_base + mem_size;
   a->max_allocs = max_allocs;
   a->block_align = align;
   a->block_shift = 0;
   while ((1u << a->block_shift) < a->block_align)
      a->block_shift++;

   // init sentinel block
   a->blocks[0].prev = a->blocks[0].next = &a->blocks[1]; // point to free block
   a->blocks[0].prev_phys = a->blocks[0].next_phys = &a->blocks[1]; // same

   // init first free block
   a->blocks[1].ptr = a->mem_base;
   a->blocks[1].prev = a->blocks[1].next = &a->blocks[0];
   a->blocks[1].prev_phys = a->blocks[1].next_phys = &a->blocks[0];
   a->blocks[1].is_free = 1;
   a->blocks[1].size = mem_size;

   // init "unused" list
   a->unused_list = a->blocks + 2;
   for (i=2; i < max_blocks; i++) {
      a->blocks[i].is_unused = 1;
      a->blocks[i].next = a->blocks + (i + 1);
   }
   a->blocks[i].is_unused = 1;

   gfxalloc_check(a);
   debug_complete_check(a, nullptr, 0,0);
   return a;
}

static void *gfxalloc_alloc(gfx_allocator *alloc, U32 size_in_bytes)
{
   gfx_block_info *cur, *best = nullptr;
   U32 i, best_wasted = ~0u;
   U32 size = size_in_bytes;
debug_complete_check(alloc, nullptr, 0,0);
gfxalloc_check(alloc);
   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_blocks == alloc->num_alloc + alloc->num_free + alloc->num_unused);)
   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_free <= alloc->num_blocks+1);)


   // round up to multiple of our block alignment
   size = (size + alloc->block_align-1) & ~(alloc->block_align - 1);
   assert(size >= size_in_bytes);
   assert(size != 0);

   // find best fit among all free blocks. this is O(N)!
   for (cur = alloc->blocks[0].next; cur != alloc->blocks; cur = cur->next) {
      if (cur->size >= size) {
         U32 wasted = cur->size - size;
         if (wasted < best_wasted) {
            best_wasted = wasted;
            best = cur;
            if (!wasted) break; // can't get better than perfect
         }
      }
   }

   // return the best fit, if we found any suitable block
   if (best) {
debug_check_overlap(alloc->cache, best->ptr, best->size);
      // split off allocated part
      if (size != best->size)
         gfxalloc_split_free(alloc, best, size);
debug_complete_check(alloc, best->ptr, best->size,0);

      // remove from free list and add to allocated hash table
      GFXALLOC_ASSERT(best->size == size);
      gfxalloc_rem_free(alloc, best);

      i = gfxalloc_get_hash_code(alloc, best->ptr);
      best->next = alloc->hash[i];
      alloc->hash[i] = best;
      alloc->actual_bytes_free -= size;
      GFXALLOC_ASSERT(alloc->actual_bytes_free >= 0);

      GFXALLOC_IF_CHECK(++alloc->num_alloc;)
      GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_blocks == alloc->num_alloc + alloc->num_free + alloc->num_unused);)
      GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_free <= alloc->num_blocks+1);)

debug_complete_check(alloc, best->ptr, best->size,0);
gfxalloc_check(alloc);
debug_check_overlap(alloc->cache, best->ptr, best->size);
      return best->ptr;
   } else
      return nullptr; // not enough space!
}

static void gfxalloc_free(gfx_allocator *alloc, void *ptr)
{
   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_blocks == alloc->num_alloc + alloc->num_free + alloc->num_unused);)
   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_free <= alloc->num_blocks+1);)

   // find the block in the hash table
   gfx_block_info *b, *t, **prevnext;
   U32 i = gfxalloc_get_hash_code(alloc, ptr);

   prevnext = &alloc->hash[i];
   b = alloc->hash[i];

   while (b) {
      if (b->ptr == ptr) break;
      prevnext = &b->next;
      b = b->next;
   }

   if (!b) {
      GFXALLOC_ASSERT(0); // trying to free a non-allocated block
      return;
   }

debug_complete_check(alloc, b->ptr, b->size, 0);
   GFXALLOC_IF_CHECK(--alloc->num_alloc;)

   // remove it from the hash table
   *prevnext = b->next;

   alloc->actual_bytes_free += b->size;

   // merge with previous block if it's free, else add it to free list
   t = b->prev_phys;
   if (t->is_free) {
      t->size += b->size;
      t->next_phys = b->next_phys;
      t->next_phys->prev_phys = t;
      gfxalloc_push_unused(alloc, b);
      b = t;
   } else
      gfxalloc_add_free(alloc, b);

   // try to merge with next block
   t = b->next_phys;
   if (t->is_free) {
      b->size += t->size;
      b->next_phys = t->next_phys;
      t->next_phys->prev_phys = b;
      gfxalloc_rem_free(alloc, t);
      gfxalloc_push_unused(alloc, t);
   }
debug_complete_check(alloc, 0, 0, ptr);
gfxalloc_check(alloc);
   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_blocks == alloc->num_alloc + alloc->num_free + alloc->num_unused);)
   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_free <= alloc->num_blocks+1);)
}

#ifdef GDRAW_MANAGE_MEM_TWOPOOL

static rrbool gfxalloc_is_empty(gfx_allocator *alloc)
{
   gfx_block_info *first_free = alloc->blocks[0].next;

   // we want to check whether there's exactly one free block that
   // covers the entire pool.
   if (first_free == alloc->blocks) // 0 free blocks
      return false;

   if (first_free->next != alloc->blocks) // >1 free block
      return false;

   return first_free->ptr == alloc->mem_base && first_free->ptr + first_free->size == alloc->mem_end;
}

static rrbool gfxalloc_mem_contains(gfx_allocator *alloc, void *ptr)
{
   return alloc->mem_base <= (U8*)ptr && (U8*)ptr < alloc->mem_end;
}

#endif

#ifdef GDRAW_DEBUG

static void gfxalloc_dump(gfx_allocator *alloc)
{
   static const char *type[] = {
      "allocated",
      "free",
   };

   for (gfx_block_info *b = alloc->blocks[0].next_phys; b != alloc->blocks; b=b->next_phys) {
      U8 *start = b->ptr;
      U8 *end = b->ptr + b->size;
      printf("%p-%p: %s (%d bytes)\n",  start, end, type[b->is_free], b->size);
   }
}

#endif

#endif

#ifdef GDRAW_DEFRAGMENT

#define GDRAW_DEFRAGMENT_may_overlap   1  // self-overlap for individual copies is OK

// Defragmentation code for graphics memory.
// The platform implementation must provide a GPU memcpy function and handle all necessary
// synchronization. It must also adjust its resource descriptors to match the new addresses
// after defragmentation.

static void gdraw_gpu_memcpy(GDrawHandleCache *c, void *dst, void *src, U32 num_bytes);

static void gdraw_Defragment_memmove(GDrawHandleCache *c, U8 *dst, U8 *src, U32 num_bytes, U32 flags, GDrawStats *stats)
{
   if (dst == src)
      return;

   assert(num_bytes != 0);

   stats->nonzero_flags |= GDRAW_STATS_defrag;
   stats->defrag_objects += 1;
   stats->defrag_bytes += num_bytes;

   if ((flags & GDRAW_DEFRAGMENT_may_overlap) || dst + num_bytes <= src || src + num_bytes <= dst) // no problematic overlap
      gdraw_gpu_memcpy(c, dst, src, num_bytes);
   else {
      // need to copy in multiple chunks
      U32 chunk_size, pos=0;
      if (dst < src)
         chunk_size = (U32) (src - dst);
      else
         chunk_size = (U32) (dst - src);

      while (pos < num_bytes) {
         U32 amount = num_bytes - pos;
         if (amount > chunk_size) amount = chunk_size;
         gdraw_gpu_memcpy(c, dst + pos, src + pos, amount);
         pos += amount;
      }
   }
}

static rrbool gdraw_CanDefragment(GDrawHandleCache *c)
{
   // we can defragment (and extract some gain from it) if and only if there's more
   // than one free block. since gfxalloc coalesces free blocks immediately and keeps
   // them in a circular linked list, this is very easy to detect: just check if the
   // "next" pointer of the first free block points to the sentinel. (this is only
   // the case if there are 0 or 1 free blocks)
   gfx_allocator *alloc = c->alloc;
   return alloc->blocks[0].next->next != alloc->blocks;
}

static void gdraw_DefragmentMain(GDrawHandleCache *c, U32 flags, GDrawStats *stats)
{
   gfx_allocator *alloc = c->alloc;
   gfx_block_info *b, *n;
   U8 *p;
   S32 i;

   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_blocks == alloc->num_alloc + alloc->num_free + alloc->num_unused);)
   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_free <= alloc->num_blocks+1);)

   // go over all allocated memory blocks and clear the "prev" pointer
   // (unused for allocated blocks, we'll use it to store a back-pointer to the corresponding handle)
   for (b = alloc->blocks[0].next_phys; b != alloc->blocks; b=b->next_phys)
      if (!b->is_free)
         b->prev = nullptr;

   // go through all handles and store a pointer to the handle in the corresponding memory block
   for (i=0; i < c->max_handles; i++)
      if (c->handle[i].raw_ptr) {
         assert(c->handle[i].bytes != 0);
         for (b=alloc->hash[gfxalloc_get_hash_code(alloc, c->handle[i].raw_ptr)]; b; b=b->next)
            if (b->ptr == c->handle[i].raw_ptr) {
               void *block = &c->handle[i];
               b->prev = (gfx_block_info *) block;
               break;
            }

         GFXALLOC_ASSERT(b != nullptr); // didn't find this block anywhere!
      }

   // clear alloc hash table (we rebuild it during defrag)
   memset(alloc->hash, 0, sizeof(alloc->hash));

   // defragmentation proper: go over all blocks again, remove all free blocks from the physical
   // block list and compact the remaining blocks together.
   p = alloc->mem_base;
   for (b = alloc->blocks[0].next_phys; b != alloc->blocks; b=n) {
      n = b->next_phys;

      if (!b->is_free) {
         U32 h;

         // move block if necessary
         if (p != b->ptr) {
            assert(b->size != 0);
            gdraw_Defragment_memmove(c, p, b->ptr, b->size, flags, stats);
            b->ptr = p;
            assert(b->prev);
            if (b->prev)
               ((GDrawHandle *) b->prev)->raw_ptr = p;
         }

         // re-insert into hash table
         h = gfxalloc_get_hash_code(alloc, p);
         b->next = alloc->hash[h];
         alloc->hash[h] = b;

         p += b->size;
      } else {
         // free block: remove it from the physical block list
         b->prev_phys->next_phys = b->next_phys;
         b->next_phys->prev_phys = b->prev_phys;
         gfxalloc_rem_free(alloc, b);
         gfxalloc_push_unused(alloc, b);
      }
   }
   // the free list should be empty now
   assert(alloc->blocks[0].next == &alloc->blocks[0]);

   // unless all memory is allocated, we now need to add a new block for the free space at the end
   if (p != alloc->mem_end) {
      b = gfxalloc_pop_unused(alloc);

      b->ptr = p;
      b->prev_phys = alloc->blocks[0].prev_phys;
      b->next_phys = &alloc->blocks[0];
      b->prev_phys->next_phys = b;
      b->next_phys->prev_phys = b;
      b->size = alloc->mem_end - p;
      gfxalloc_add_free(alloc, b);
   }

   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_blocks == alloc->num_alloc + alloc->num_free + alloc->num_unused);)
   GFXALLOC_IF_CHECK(GFXALLOC_ASSERT(alloc->num_free <= alloc->num_blocks+1);)
}

#endif

#ifdef GDRAW_MANAGE_MEM_TWOPOOL

// Defragmentation code for graphics memory, using two-pool strategy.
//
// The platform implementation must provide a GPU memcpy function and handle
// all necessary synchronization. It must also adjust its resource descriptors
// to match the new addresses after defragmentation.
//
// The high concept for two-pool is that we can't update the resource pools
// mid-frame; instead, while preparing for a frame, we need to produce a memory
// configuration that is suitable for rendering a whole frame at once (in
// contrast to our normal incremental strategy, where we can decide to
// defragment mid-frame if things are getting desperate). This is for tiled
// renderers.
//
// Two-pool works like this:
// - As the name suggests, each handle cache has two memory pools and corresponding backing
//   allocators. The currently used allocator, "alloc", and a second allocator, "alloc_other".
// - Any resource used in a command buffer gets locked and *stays locked* until we're done
//   preparing that command buffer (i.e. no unlocking after every draw as in the normal
//   incremental memory management).
// - All allocations happen from "alloc", always. We mostly do our normal LRU cache freeing
//   to make space when required.
// - We can still run out of space (no surprise) and get into a configuration where we have
//   to defragment. This is the only tricky part, and where the second pool comes in. To
//   defragment, we switch the roles of "alloc" and "alloc_other", and allocate new backing
//   storage for all currently "locked" and "pinned" resources (i.e. everything we've used
//   in the currently pending frame). 
// - In general, we have the invariant that all resources we're using for batches we're
//   working on must be in the "alloc" (fresh) pool, not in the "other" (stale) pool.
//   Therefore, after a defragment/pool switch, any "live" resource (which means it's
//   present in the stale pool) has to be copied to the "fresh" pool as it's getting
//   locked to maintain this invariant.
//
// What this does is give us a guarantee that any given frame either only
// references resources in one pool (the common case), or does a defragment, in
// which case it looks like this:
//
//   +------------------------------+
//   |                              |
//   |                              |  pool A is fresh (=alloc), pool B is stale (=alloc_other)
//   |                              |  all resources referenced in here are in pool A
//   |                              |
//   |                              |
//   |                              |
//   +------------------------------+  <-- defragment! pools flip roles here
//   |                              |
//   |                              |
//   |                              |  pool B is fresh (=alloc), pool A is stale (=alloc_other)
//   |                              |  all resources referenced in here are in pool B
//   |                              |
//   +------------------------------+
//
// Now, at the end of the frame, we need to decide what to do with the
// resources that remain "live" (i.e. they're in the old pool but weren't
// referenced in the current frame so they didn't get copied). As of this
// writing, we simply free them, to maximize the amount of free memory in the
// new pool (and hopefully minimize the chance that we'll have to defragment
// again soon). It would also be possible to copy some of them though, assuming
// there's enough space.
//
// Freeing resources is an interesting case. When the CPU side of GDraw does a
// "free", we can't immediately reclaim the resource memory, since the GPU will
// generally still have outstanding commands that reference that resource. So
// our freed resources first enter the "Dead" state and only actually get freed
// once the GPU is done with them. What this means is that the list of
// resources in the "dead" state can end up holding references to both the
// fresh and the stale pool; the free implementation needs to be aware of this
// and return the memory to the right allocator.
//
// When we defragment, it's important to make sure that the pool we're flipping
// to is actually empty. What this means is that right before a defragment, we
// need to wait for all stale "dead" resources to actually become free. If the
// last defragment was several frames ago, this is fast - we haven't generated
// any new commands referencing the stale resources in several frames, so most
// likely they're all immediately free-able.  By contrast, if we just
// defragmented last frame, this will be a slow operation since we need to wait
// for the GPU pipeline to drain - but if you're triggering defragments in
// several consecutive frames, you're thrashing the resource pools badly and
// are getting really bad performance anyway.

static void gdraw_gpu_memcpy(GDrawHandleCache *c, void *dst, void *src, U32 num_bytes);
static void gdraw_gpu_wait_for_transfer_completion();
static void gdraw_resource_moved(GDrawHandle *t);

static rrbool gdraw_CanDefragment(GDrawHandleCache *c)
{
   // we can defragment (and extract some gain from it) if and only if there's more
   // than one free block. since gfxalloc coalesces free blocks immediately and keeps
   // them in a circular linked list, this is very easy to detect: just check if the
   // "next" pointer of the first free block points to the sentinel. (this is only
   // the case if there are 0 or 1 free blocks)
   gfx_allocator *alloc = c->alloc;
   if (!c->alloc_other) // if we don't have a second pool, we can't defrag at all.
      return false;
   return alloc->blocks[0].next->next != alloc->blocks;
}

static rrbool gdraw_MigrateResource(GDrawHandle *t, GDrawStats *stats)
{
   GDrawHandleCache *c = t->cache;
   void *ptr = nullptr;

   assert(t->state == GDRAW_HANDLE_STATE_live || t->state == GDRAW_HANDLE_STATE_locked || t->state == GDRAW_HANDLE_STATE_pinned);
   // anything we migrate should be in the "other" (old) pool
   assert(gfxalloc_mem_contains(c->alloc_other, t->raw_ptr));

   ptr = gfxalloc_alloc(c->alloc, t->bytes);
   if (ptr) {
      // update stats
      stats->nonzero_flags |= GDRAW_STATS_defrag;
      stats->defrag_objects += 1;
      stats->defrag_bytes += t->bytes;

      // copy contents to new storage
      gdraw_gpu_memcpy(c, ptr, t->raw_ptr, t->bytes);

      // free old storage
      gfxalloc_free(c->alloc_other, t->raw_ptr);

      // adjust pointers to point to new location
      t->raw_ptr = ptr;
      gdraw_resource_moved(t);

      return true;
   } else
      return false;
}

static rrbool gdraw_MigrateAllResources(GDrawHandle *sentinel, GDrawStats *stats)
{
   GDrawHandle *h;
   for (h = sentinel->next; h != sentinel; h = h->next) {
      if (!gdraw_MigrateResource(h, stats))
         return false;
   }
   return true;
}

static rrbool gdraw_TwoPoolDefragmentMain(GDrawHandleCache *c, GDrawStats *stats)
{
   gfx_allocator *t;

   // swap allocators
   t = c->alloc;
   c->alloc = c->alloc_other;
   c->alloc_other = t;

   // immediately migrate all currently pinned and locked resources
   rrbool ok = true;
   ok = ok && gdraw_MigrateAllResources(&c->state[GDRAW_HANDLE_STATE_pinned], stats);
   ok = ok && gdraw_MigrateAllResources(&c->state[GDRAW_HANDLE_STATE_locked], stats);

   return ok;
}

static rrbool gdraw_StateListIsEmpty(GDrawHandle *head)
{
   // a list is empty when the head sentinel is the only node
   return head->next == head;
}

static void gdraw_CheckAllPointersUpdated(GDrawHandle *head)
{
#ifdef GDRAW_DEBUG
   GDrawHandle *h;
   for (h = head->next; h != head; h = h->next) {
      assert(gfxalloc_mem_contains(h->cache->alloc, h->raw_ptr));
   }
#endif
}

static void gdraw_PostDefragmentCleanup(GDrawHandleCache *c, GDrawStats *stats)
{
   // if we defragmented during this scene, this is the spot where
   // we need to nuke all references to resources that weren't
   // carried over into the new pool.
   if (c->did_defragment) {
      GDrawHandle *h;

      // alloc list should be empty at this point
      assert(gdraw_StateListIsEmpty(&c->state[GDRAW_HANDLE_STATE_alloc]));

      // free all remaining live resources (these are the resources we didn't
      // touch this frame, hence stale)
      h = &c->state[GDRAW_HANDLE_STATE_live];
      while (!gdraw_StateListIsEmpty(h))
         gdraw_res_free(h->next, stats);

      // "live" is now empty, and we already checked that "alloc" was empty
      // earlier. "dead" may hold objects on the old heap still (that were freed
      // before we swapped allocators). "user owned" is not managed by us.
      // that leaves "locked" and "pinned" resources, both of which better be
      // only pointing into the new heap now!
      gdraw_CheckAllPointersUpdated(&c->state[GDRAW_HANDLE_STATE_locked]);
      gdraw_CheckAllPointersUpdated(&c->state[GDRAW_HANDLE_STATE_pinned]);

      gdraw_gpu_wait_for_transfer_completion();
   }
}

#endif

// Image processing code

// Compute average of 4 RGBA8888 pixels passed as U32.
// Variables are named assuming the values are stored as big-endian, but all bytes
// are treated equally, so this code will work just fine on little-endian data.
static U32 gdraw_Avg4_rgba8888(U32 p0, U32 p1, U32 p2, U32 p3)
{
   U32 mask = 0x00ff00ff;
   U32 bias = 0x00020002;
   
   U32 gasum = ((p0 >> 0) & mask) + ((p1 >> 0) & mask) + ((p2 >> 0) & mask) + ((p3 >> 0) & mask) + bias;
   U32 rbsum = ((p0 >> 8) & mask) + ((p1 >> 8) & mask) + ((p2 >> 8) & mask) + ((p3 >> 8) & mask) + bias;

   return ((gasum >> 2) & mask) | ((rbsum << 6) & ~mask);
}

// Compute average of 2 RGBA8888 pixels passed as U32
static U32 gdraw_Avg2_rgba8888(U32 p0, U32 p1)
{
   return (p0 | p1) - (((p0 ^ p1) >> 1) & 0x7f7f7f7f);
}

// 2:1 downsample in both horizontal and vertical direction, for one line.
// width is width of destination line.
static void gdraw_Downsample_2x2_line(U8 *dst, U8 *line0, U8 *line1, U32 width, U32 bpp)
{
   U32 x;
   if (bpp == 4) {
      U32 *in0 = (U32 *) line0;
      U32 *in1 = (U32 *) line1;
      U32 *out = (U32 *) dst;
      for (x=0; x < width; x++, in0 += 2, in1 += 2)
         *out++ = gdraw_Avg4_rgba8888(in0[0], in0[1], in1[0], in1[1]);
   } else if (bpp == 1) {
      for (x=0; x < width; x++, line0 += 2, line1 += 2)
         *dst++ = (line0[0] + line0[1] + line1[0] + line1[1] + 2) / 4;
   } else
      RR_BREAK();
}

// 2:1 downsample in horizontal but not vertical direction.
static void gdraw_Downsample_2x1_line(U8 *dst, U8 *src, U32 width, U32 bpp)
{
   U32 x;
   if (bpp == 4) {
      U32 *in = (U32 *) src;
      U32 *out = (U32 *) dst;
      for (x=0; x < width; x++, in += 2)
         *out++ = gdraw_Avg2_rgba8888(in[0], in[1]);
   } else if (bpp == 1) {
      for (x=0; x < width; x++, src += 2)
         *dst++ = (src[0] + src[1] + 1) / 2;
   } else
      RR_BREAK();
}

// 2:1 downsample in vertical but not horizontal direction.
static void gdraw_Downsample_1x2(U8 *dst, S32 dstpitch, U8 *src, S32 srcpitch, U32 height, U32 bpp)
{
   U32 y;
   if (bpp == 4) {
      for (y=0; y < height; y++, dst += dstpitch, src += 2*srcpitch)
         *((U32 *) dst) = gdraw_Avg2_rgba8888(*((U32 *) src), *((U32 *) (src + srcpitch)));
   } else if (bpp == 1) {
      for (y=0; y < height; y++, dst += dstpitch, src += 2*srcpitch)
         *dst = (src[0] + src[srcpitch] + 1) / 2;
   } else
      RR_BREAK();
}

// 2:1 downsample (for mipmaps)
// dst: Pointer to destination buffer
// dstpitch: Pitch for destination buffer
// width: Width of *destination* image (i.e. downsampled version)
// height: Height of *destination* image (i.e. downsampled version)
// src: Pointer to source buffer
// srcpitch: Pitch of source buffer
// bpp: Bytes per pixel for image data
//
// can be used for in-place resizing if src==dst and dstpitch <= srcpitch!
static GDRAW_MAYBE_UNUSED void gdraw_Downsample(U8 *dst, S32 dstpitch, U32 width, U32 height, U8 *src, S32 srcpitch, U32 bpp)
{
   U32 y;
   assert(bpp == 1 || bpp == 4);

   // @TODO gamma?
   if (!height) // non-square texture, height was reduced to 1 in a previous step
      gdraw_Downsample_2x1_line(dst, src, width, bpp);
   else if (!width) // non-square texture, width was reduced to 1 in a previous step
      gdraw_Downsample_1x2(dst, dstpitch, src, srcpitch, height, bpp);
   else {
      for (y=0; y < height; y++) {
         gdraw_Downsample_2x2_line(dst, src, src + srcpitch, width, bpp);
         dst += dstpitch;
         src += 2*srcpitch;
      }
   }
}

#ifndef GDRAW_NO_STREAMING_MIPGEN

#define GDRAW_MAXMIPS 16 // maximum number of mipmaps supported.

typedef struct GDrawMipmapContext {
   U32 width; // width of the texture being mipmapped
   U32 height; // height of the texture being mipmapped
   U32 mipmaps; // number of mipmaps
   U32 bpp; // bytes per pixel

   U32 partial_row; // bit N: is mipmap N currently storing a partial row?
   U32 bheight; // height of the buffer at miplevel 0
   U8 *pixels[GDRAW_MAXMIPS];
   U32 pitch[GDRAW_MAXMIPS];
} GDrawMipmapContext;

static rrbool gdraw_MipmapBegin(GDrawMipmapContext *c, U32 width, U32 height, U32 mipmaps, U32 bpp, U8 *buffer, U32 buffer_size)
{
   U32 i;
   U8 *p;

   if (mipmaps > GDRAW_MAXMIPS)
      return false;

   c->width = width;
   c->height = height;
   c->mipmaps = mipmaps;
   c->bpp = bpp;
   c->partial_row = 0;

   // determine how many lines to buffer
   // we try to use roughly 2/3rds of the buffer for the first miplevel (less than 3/4 since with our
   // partial line buffers, we have extra buffer space for lower mip levels).
   c->bheight = (2 * buffer_size) / (3 * width * bpp);

   // round down to next-smaller power of 2 (in case we need to swizzle; swizzling works on pow2-sized blocks)
   while (c->bheight & (c->bheight-1)) // while not a power of 2...
      c->bheight &= c->bheight - 1; // clear least significant bit set

   // then keep lowering the number of buffered lines until they fit (or we reach zero, i.e. it doesn't fit)
   while (c->bheight) {
      p = buffer;
      for (i=0; i < c->mipmaps; i++) {
         U32 mw = c->width >> i;
         U32 bh = c->bheight >> i;
         if (!mw) mw++;
         if (!bh) mw *= 2, bh++; // need space for line of previous miplevel

         c->pixels[i] = p;
         c->pitch[i] = mw * bpp;
         p += c->pitch[i] * bh;
      }

      // if it fits, we're done
      if (p <= buffer + buffer_size) {
         if (c->bheight > height) // buffer doesn't need to be larger than the image!
            c->bheight = height;
         return true;
      }

      // need to try a smaller line buffer...
      c->bheight >>= 1;
   }

   // can't fit even one line into our buffer. ouch!
   return false;
}

// returns true if there was data generated for this miplevel, false otherwise.
static rrbool gdraw_MipmapAddLines(GDrawMipmapContext *c, U32 level)
{
   U32 bw,bh;

   assert(level > 0); // doesn't make sense to call this on level 0
   if (level == 0 || level >= c->mipmaps)
      return false; // this level doesn't exist

   bw = c->width >> level;    // buffer width at this level
   bh = c->bheight >> level;  // buffer height at this level

   if (bh) { // we can still do regular downsampling
      gdraw_Downsample(c->pixels[level], c->pitch[level], bw, bh, c->pixels[level-1], c->pitch[level-1], c->bpp);
      return true;
   } else if (c->height >> level) { // need to buffer partial lines, but still doing vertical 2:1 downsampling
      if ((c->partial_row ^= (1 << level)) & (1 << level)) { // no buffered partial row for this miplevel yet, make one
         memcpy(c->pixels[level], c->pixels[level-1], bw * 2 * c->bpp);
         return false;
      } else { // have one buffered row, can generate output pixels
         gdraw_Downsample_2x2_line(c->pixels[level], c->pixels[level], c->pixels[level-1], bw, c->bpp);
         return true;
      }
   } else { // finish off with a chain of Nx1 miplevels
      gdraw_Downsample_2x1_line(c->pixels[level], c->pixels[level-1], bw, c->bpp);
      return true;
   }
}

#endif // GDRAW_NO_STREAMING_MIPGEN

#ifdef GDRAW_CHECK_BLOCK
static void check_block_alloc(gfx_allocator *alloc, void *ptr, rrbool allocated)
{
   int i,n=0,m=0;
   for (i=0; i < GFXALLOC_HASH_SIZE; ++i) {
      gfx_block_info *b = alloc->hash[i];
      while (b) {
         if (b->ptr == ptr)
            ++n;
         b = b->next;
      }
   }
   gfx_block_info *b = alloc->blocks[0].next;
   while (b != &alloc->blocks[0]) {
      if (b->ptr == ptr)
         ++m;
      b = b->next;
   }
   if (allocated)
      assert(n == 1 && m == 0);
   else
      assert(n == 0 && m == 1);
}
#else
#define check_block_alloc(a,p,f)
#endif

#ifdef GDRAW_BUFFER_RING

////////////////////////////////////////////////////////////////////////
//
//   Buffer ring
//

// Implements a dynamic buffer backed by multiple physical buffers, with
// the usual append-only, DISCARD/NOOVERWRITE semantics.
//
// This can be used for dynamic vertex buffers, constant buffers, etc.
#define GDRAW_BUFRING_MAXSEGS 4     // max number of backing segments

typedef struct gdraw_bufring_seg {
   struct gdraw_bufring_seg *next;  // next segment in ring
   U8 *data;                        // pointer to the allocation
   GDrawFence fence;                // fence for this segment
   U32 used;                        // number of bytes used
} gdraw_bufring_seg;

typedef struct gdraw_bufring {
   gdraw_bufring_seg *cur;          // active ring segment
   U32 seg_size;                    // size of one segment
   U32 align;                       // alignment of segment allocations
   gdraw_bufring_seg all_segs[GDRAW_BUFRING_MAXSEGS];
} gdraw_bufring;

// forwards
static GDrawFence put_fence();
static void wait_on_fence(GDrawFence fence);

static void gdraw_bufring_init(gdraw_bufring * RADRESTRICT ring, void *ptr, U32 size, U32 nsegs, U32 align)
{
   U32 i, seg_size;

   ring->seg_size = 0;
   if (!ptr || nsegs < 1 || size < nsegs * align) // bail if no ring buffer memory or too small
      return;

   if (nsegs > GDRAW_BUFRING_MAXSEGS)
      nsegs = GDRAW_BUFRING_MAXSEGS;

   // align needs to be a positive power of two
   assert(align >= 1 && (align & (align - 1)) == 0);

   // buffer really needs to be properly aligned
   assert(((UINTa)ptr & (align - 1)) == 0);

   seg_size = (size / nsegs) & ~(align - 1);
   for (i=0; i < nsegs; ++i) {
      ring->all_segs[i].next = &ring->all_segs[(i + 1) % nsegs];
      ring->all_segs[i].data = (U8 *) ptr + i * seg_size;
      ring->all_segs[i].fence.value = 0;
      ring->all_segs[i].used = 0;
   }

   ring->cur = ring->all_segs;
   ring->seg_size = seg_size;
   ring->align = align;
}

static void gdraw_bufring_shutdown(gdraw_bufring * RADRESTRICT ring)
{
   ring->cur = nullptr;
   ring->seg_size = 0;
}

static void *gdraw_bufring_alloc(gdraw_bufring * RADRESTRICT ring, U32 size, U32 align)
{
   U32 align_up;
   gdraw_bufring_seg *seg;

   if (size > ring->seg_size)
      return nullptr; // nope, won't fit

   assert(align <= ring->align);

   // check if it fits in the active segment first
   seg = ring->cur;
   align_up = (seg->used + align - 1) & -align;

   if ((align_up + size) <= ring->seg_size) {
      void *ptr = seg->data + align_up;
      seg->used = align_up + size;
      return ptr;
   }

   // doesn't fit, we have to start a new ring segment.
   seg->fence = put_fence();

   // switch to the next segment, wait till GPU is done with it
   seg = ring->cur = seg->next;
   wait_on_fence(seg->fence);

   // allocate from the new segment. we assume that segment offsets
   // satisfy the highest alignment requirements we ever ask for!
   seg->used = size;
   return seg->data;
}

#endif

////////////////////////////////////////////////////////////////////////
//
//   General resource manager
//

#ifndef GDRAW_FENCE_FLUSH
#define GDRAW_FENCE_FLUSH()
#endif

#ifdef GDRAW_MANAGE_MEM
// functions the platform must implement
#ifndef GDRAW_BUFFER_RING // avoid "redundant redeclaration" warning
static void wait_on_fence(GDrawFence fence);
#endif
static rrbool is_fence_pending(GDrawFence fence);
static void gdraw_defragment_cache(GDrawHandleCache *c, GDrawStats *stats);

// functions we implement
static void gdraw_res_reap(GDrawHandleCache *c, GDrawStats *stats);
#endif

// If GDRAW_MANAGE_MEM is not #defined, this needs to perform the
// actual free using whatever API we're targeting.
//
// If GDRAW_MANAGE_MEM is #defined, the shared code handles the
// memory management part, but you might still need to update
// your state caching.
static void api_free_resource(GDrawHandle *r);

// Actually frees a resource and releases all allocated resources
static void gdraw_res_free(GDrawHandle *r, GDrawStats *stats)
{
   assert(r->state == GDRAW_HANDLE_STATE_live || r->state == GDRAW_HANDLE_STATE_locked || r->state == GDRAW_HANDLE_STATE_dead ||
      r->state == GDRAW_HANDLE_STATE_pinned || r->state == GDRAW_HANDLE_STATE_user_owned);

#ifdef GDRAW_MANAGE_MEM
   GDRAW_FENCE_FLUSH();

   // make sure resource isn't in use before we actually free the memory
   wait_on_fence(r->fence);
   if (r->raw_ptr) {
#ifndef GDRAW_MANAGE_MEM_TWOPOOL
      gfxalloc_free(r->cache->alloc, r->raw_ptr);
#else
      GDrawHandleCache *c = r->cache;
      if (gfxalloc_mem_contains(c->alloc, r->raw_ptr))
         gfxalloc_free(c->alloc, r->raw_ptr);
      else {
         assert(gfxalloc_mem_contains(c->alloc_other, r->raw_ptr));
         gfxalloc_free(c->alloc_other, r->raw_ptr);
      }
#endif
   }
#endif

   api_free_resource(r);

   stats->nonzero_flags |= GDRAW_STATS_frees;
   stats->freed_objects += 1;
   stats->freed_bytes += r->bytes;

   gdraw_HandleCacheFree(r);
}

// Frees the LRU resource in the given cache.
static rrbool gdraw_res_free_lru(GDrawHandleCache *c, GDrawStats *stats)
{
   GDrawHandle *r = gdraw_HandleCacheGetLRU(c);
   if (!r) return false;

   if (c->is_vertex && r->owner) // check for r->owner since it may already be killed (if player destroyed first)
      IggyDiscardVertexBufferCallback(r->owner, r);

   // was it referenced since end of previous frame (=in this frame)?
   // if some, we're thrashing; report it to the user, but only once per frame.
   if (c->prev_frame_end.value < r->fence.value && !c->is_thrashing) {
      IggyGDrawSendWarning(nullptr, c->is_vertex ? "GDraw Thrashing vertex memory" : "GDraw Thrashing texture memory");
      c->is_thrashing = true;
   }

   gdraw_res_free(r, stats);
   return true;
}

static void gdraw_res_flush(GDrawHandleCache *c, GDrawStats *stats)
{
   c->is_thrashing = true; // prevents warnings being generated from free_lru
   gdraw_HandleCacheUnlockAll(c);
   while (gdraw_res_free_lru(c, stats))
      ;
}

static GDrawHandle *gdraw_res_alloc_outofmem(GDrawHandleCache *c, GDrawHandle *t, char const *failed_type)
{
   if (t)
      gdraw_HandleCacheAllocateFail(t);
   IggyGDrawSendWarning(nullptr, c->is_vertex ? "GDraw Out of static vertex buffer %s" : "GDraw Out of texture %s", failed_type);
   return nullptr;
}

#ifndef GDRAW_MANAGE_MEM

static GDrawHandle *gdraw_res_alloc_begin(GDrawHandleCache *c, S32 size, GDrawStats *stats)
{
   GDrawHandle *t;
   if (size > c->total_bytes)
      gdraw_res_alloc_outofmem(c, nullptr, "memory (single resource larger than entire pool)");
   else {
      // given how much data we're going to allocate, throw out
      // data until there's "room" (this basically lets us use
      // managed memory and just bound our usage, without actually
      // packing it and being exact)
      while (c->bytes_free < size) {
         if (!gdraw_res_free_lru(c, stats)) {
            gdraw_res_alloc_outofmem(c, nullptr, "memory");
            break;
         }
      }
   }

   // now try to allocate a handle
   t = gdraw_HandleCacheAllocateBegin(c);
   if (!t) {
      // it's possible we have no free handles, because all handles
      // are in use without exceeding the max storage above--in that
      // case, just free one texture to give us a free handle (ideally
      // we'd trade off cost of regenerating)
      if (gdraw_res_free_lru(c, stats)) {
         t = gdraw_HandleCacheAllocateBegin(c);
         if (t == nullptr) {
            gdraw_res_alloc_outofmem(c, nullptr, "handles");
         }
      }
   }
   return t;
}

#else

// Returns whether this resource holds pointers to one of the GDraw-managed
// pools.
static rrbool gdraw_res_is_managed(GDrawHandle *r)
{
   return r->state == GDRAW_HANDLE_STATE_live ||
      r->state == GDRAW_HANDLE_STATE_locked ||
      r->state == GDRAW_HANDLE_STATE_dead ||
      r->state == GDRAW_HANDLE_STATE_pinned;
}

// "Reaps" dead resources. Even if the user requests that a
// resource be freed, it might still be in use in a pending
// command buffer. So we can't free the associated memory
// immediately; instead, we flag the resource as "dead" and
// periodically check whether we can actually free the
// pending memory of dead resources ("reap" them).
static void gdraw_res_reap(GDrawHandleCache *c, GDrawStats *stats)
{
   GDrawHandle *sentinel = &c->state[GDRAW_HANDLE_STATE_dead];
   GDrawHandle *t;
   GDRAW_FENCE_FLUSH();

   // reap all dead resources that aren't in use anymore
   while ((t = sentinel->next) != sentinel && !is_fence_pending(t->fence))
      gdraw_res_free(t, stats);
}

// "Kills" a resource. This means GDraw won't use it anymore
// (it's dead), but there might still be outstanding references
// to it in a pending command buffer, so we can't physically
// free the associated memory until that's all processed.
static void gdraw_res_kill(GDrawHandle *r, GDrawStats *stats)
{
   GDRAW_FENCE_FLUSH(); // dead list is sorted by fence index - make sure all fence values are current.

   r->owner = nullptr;
   gdraw_HandleCacheInsertDead(r);
   gdraw_res_reap(r->cache, stats);
}

static GDrawHandle *gdraw_res_alloc_begin(GDrawHandleCache *c, S32 size, GDrawStats *stats)
{
   GDrawHandle *t;
   void *ptr = nullptr;

   gdraw_res_reap(c, stats); // NB this also does GDRAW_FENCE_FLUSH();
   if (size > c->total_bytes)
      return gdraw_res_alloc_outofmem(c, nullptr, "memory (single resource larger than entire pool)");
   
   // now try to allocate a handle
   t = gdraw_HandleCacheAllocateBegin(c);
   if (!t) {
      // it's possible we have no free handles, because all handles
      // are in use without exceeding the max storage above--in that
      // case, just free one texture to give us a free handle (ideally
      // we'd trade off cost of regenerating)
      gdraw_res_free_lru(c, stats);
      t = gdraw_HandleCacheAllocateBegin(c);
      if (!t)
         return gdraw_res_alloc_outofmem(c, nullptr, "handles");
   }

   // try to allocate first
   if (size) {
      ptr = gfxalloc_alloc(c->alloc, size);
      if (!ptr) {
         // doesn't currently fit. try to free some allocations to get space to breathe.
         S32 want_free = RR_MAX(size + (size / 2), GDRAW_MIN_FREE_AMOUNT);
         if (want_free > c->total_bytes)
            want_free = size; // okay, *really* big resource, just try to allocate its real size

         // always keep freeing textures until want_free bytes are free.
         while (c->alloc->actual_bytes_free < want_free) {
            if (!gdraw_res_free_lru(c, stats))
               return gdraw_res_alloc_outofmem(c, t, "memory");
         }

         // now, keep trying to allocate and free some more memory when it still doesn't fit
         while (!(ptr = gfxalloc_alloc(c->alloc, size))) {
            if (c->alloc->actual_bytes_free >= 3 * size || // if we should have enough free bytes to satisfy the request by now
               (c->alloc->actual_bytes_free >= size && size * 2 >= c->total_bytes)) // or the resource is very big and the alloc doesn't fit
            {
               // before we actually consider defragmenting, we want to free all stale resources (not
               // referenced in the previous 2 frames). and if that frees up enough memory so we don't have
               // to defragment, all the better!
               // also, never defragment twice in a frame, just assume we're thrashing when we get in that
               // situation and free up as much as possible.
               if (!c->did_defragment &&
                   c->prev_frame_start.value <= c->handle->fence.value) {

                  // defragment.
   defrag:
                  if (gdraw_CanDefragment(c)) { // only try defrag if it has a chance of helping.
                     gdraw_defragment_cache(c, stats);
                     c->did_defragment = true;
                  }
                  ptr = gfxalloc_alloc(c->alloc, size);
                  if (!ptr)
                     return gdraw_res_alloc_outofmem(c, t, "memory (fragmentation)");
                  break;
               }
            }

            // keep trying to free some more
            if (!gdraw_res_free_lru(c, stats)) {
               if (c->alloc->actual_bytes_free >= size) // nothing left to free but we should be good - defrag again, even if it's the second time in a frame
                  goto defrag;

               return gdraw_res_alloc_outofmem(c, t, "memory");
            }
         }
      }
   }

   t->fence.value = 0; // hasn't been used yet
   t->raw_ptr = ptr;
   return t;
}

#endif
