/*
 * ======================================================================
 *
 * This file is part of Combat, the Tcl interface for CORBA
 * Copyright (c) Frank Pilhofer
 *
 * ======================================================================
 */
 
/*
 * Generic ORB Event handler that uses only CORBA compliant methods.
 *
 * Unfortunately, CORBA does not allow asynchronous handling (callbacks)
 * for DII requests. So we must poll the ORB once in a while to handle
 * incoming data, using CORBA::ORB::work_pending() and perform_work().
 */

#include "combat.h"
#include <assert.h>

char * combat_event_corba_id = "$Id: event-corba.cc,v 1.5 2001/05/14 13:07:47 fp Exp $";

/*
 * Poll Delay - sleep this time between ORB polls (in microseconds)
 */

static const unsigned long PollDelay = 100000;

extern "C" {

struct Combat_CorbaEvent {
  struct Tcl_Event ev;
  Combat::Context * ctx;
};

static int
Combat_HandleCorbaEvent (Tcl_Event * evPtr, int flags)
{
  assert (flags & TCL_FILE_EVENTS);

  Combat_CorbaEvent * ev = (Combat_CorbaEvent *) evPtr;
  Combat::Context * ctx = ev->ctx;

  /*
   * Do some ORB work
   */

  if (Combat::GlobalData->orb->work_pending()) {
    Combat::GlobalData->orb->perform_work ();
  }

  /*
   * If any asynchronous DII requests have finished, try all callbacks.
   */

  bool ready;

#ifdef HAVE_EXCEPTIONS
  try {
#endif
    ready = Combat::GlobalData->orb->poll_next_response();
#ifdef HAVE_EXCEPTIONS
  } catch (CORBA::BAD_INV_ORDER &) {
    ready = false;
  }
#endif

  if (ready) {
    /*
     * Call PollResult on AsyncOps so that the finished request is
     * removed from the ORBs queue and won't cause continous events
     * within SetupCorbaEvents
     */

    Combat::Context::RequestTable::iterator el = ctx->AsyncOps.begin ();
    while (el != ctx->AsyncOps.end()) {
      (*el++).second->PollResult();
    }

    /*
     * PerformCallback is supposed to call corba::request get, which
     * alters our vector<>, so the iterator becomes invalid after
     * executing a callback
     */

    el = ctx->CbOps.begin ();
    while (el != ctx->CbOps.end()) {
      if ((*el).second->PollResult()) {
	(*el).second->PerformCallback();
	el = ctx->CbOps.begin();
      }
      else {
	el++;
      }
    }
  }

  return 1;
}

static void
Combat_SetupCorbaEvents (ClientData clientData, int flags)
{
  if (!(flags & TCL_FILE_EVENTS)) {
    return;
  }

  Tcl_Time tm;
  bool ready;

#ifdef HAVE_EXCEPTIONS
  try {
#endif
    ready = Combat::GlobalData->orb->work_pending() ||
      Combat::GlobalData->orb->poll_next_response();
#ifdef HAVE_EXCEPTIONS
  } catch (CORBA::BAD_INV_ORDER &) {
    ready = false;
  }
#endif

  if (ready) {
    tm.sec  = 0;
    tm.usec = 0;
  }
  else {
    tm.sec  = PollDelay / 1000000;
    tm.usec = PollDelay % 1000000;
  }

  Tcl_SetMaxBlockTime (&tm);
}

static void
Combat_CheckCorbaEvents (ClientData clientData, int flags)
{
  if (!(flags & TCL_FILE_EVENTS)) {
    return;
  }

  bool ready;

#ifdef HAVE_EXCEPTIONS
  try {
#endif
    ready = Combat::GlobalData->orb->work_pending() ||
      Combat::GlobalData->orb->poll_next_response();
#ifdef HAVE_EXCEPTIONS
  } catch (CORBA::BAD_INV_ORDER &) {
    ready = false;
  }
#endif

  if (ready) {
    Combat_CorbaEvent * ev =
      (Combat_CorbaEvent *) Tcl_Alloc (sizeof (Combat_CorbaEvent));
    Combat::Context * ctx = (Combat::Context *) clientData;
    ev->ev.proc = Combat_HandleCorbaEvent;
    ev->ctx     = ctx;
    Tcl_QueueEvent ((struct Tcl_Event *) ev, TCL_QUEUE_TAIL);
  }
}

};

/*
 * Register Event Handler with Tcl
 */

int
Combat::SetupORBEventHandler (Tcl_Interp *, Combat::Context * ctx)
{
  Tcl_CreateEventSource (Combat_SetupCorbaEvents,
			 Combat_CheckCorbaEvents,
			 (ClientData) ctx);
  return TCL_OK;
}
