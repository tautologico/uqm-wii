/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <network.h>      // Wii network functions from libogc
#undef TRUE
#undef FALSE

#include "uqmlog.h"
#include "loginternal.h"
#include "msgbox.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include "libs/threadlib.h"


#ifndef MAX_LOG_ENTRY_SIZE
#	define MAX_LOG_ENTRY_SIZE 1024
#endif

#ifndef MAX_LOG_ENTRIES
#	define MAX_LOG_ENTRIES 128
#endif

typedef char log_Entry[MAX_LOG_ENTRY_SIZE];

// static buffers in case we run out of memory
static log_Entry queue[MAX_LOG_ENTRIES];
static log_Entry msgNoThread;
static char msgBuf[16384];

static int maxLevel = log_Error;
static int maxStreamLevel = log_All;
static int maxDisp = 10;
static int qtotal = 0;
static int qhead = 0;
static int qtail = 0;
static volatile bool noThreadReady = false;
static bool showBox = true;
static bool errorBox = true;

FILE *streamOut;

static void exitCallback (void);
static void displayLog (bool isError);

static s32 socket; 
static const char * server_address = "192.168.1.110";
static int port_number = 8877;


static void removeExcess(int room)
{
	room = maxDisp - room;
	if (room < 0)
		room = 0;

	for ( ; qtotal > room; --qtotal, ++qtail)
		;
	qtail %= MAX_LOG_ENTRIES;
}

static int acquireSlot(void)
{
	int slot;

	removeExcess (1);
	slot = qhead;
	qhead = (qhead + 1) % MAX_LOG_ENTRIES;
	++qtotal;
	
	return slot;
}

// queues the non-threaded message when present
static void queueNonThreaded(void)
{
	int slot;

	// This is not perfect. A race condition still exists
	// between buffering the no-thread message and setting
	// the noThreadReady flag. Neither does this prevent
	// the fully or partially overwritten message (by
	// another competing thread). But it is 'good enough'
	if (!noThreadReady)
		return;
	noThreadReady = false;

	slot = acquireSlot ();
	memcpy (queue[slot], msgNoThread, sizeof (msgNoThread));
}

void init_network(void)
{
	s32 result = -1;
	int tries;

	for (tries = 0; tries < 10 && result < 0; ++tries) {
		net_deinit();
		result = net_init();
	}

	if (result < 0) {
		net_deinit();
		fprintf(stderr, "Could not initialize network, exiting.");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	if (inet_aton(server_address, &(addr.sin_addr)) == 0) {
		fprintf(stderr, "Error transforming server address");
		exit(EXIT_FAILURE);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_number);

	socket = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	if (socket < 0) {
		fprintf(stderr, "Error creating socket");
		exit(EXIT_FAILURE);
	}

	if (net_connect(socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "Error connecting to server");
		exit(EXIT_FAILURE);
	}

	char msg[512];
	sprintf(msg, "Connected after %d tries\n", tries);
	net_send(socket, msg, strlen(msg), 0);
}

void log_init(int max_lines)
{
	int i;

	maxDisp = max_lines;
	streamOut = stderr;

	// pre-term queue strings
	for (i = 0; i < MAX_LOG_ENTRIES; ++i)
		queue[i][MAX_LOG_ENTRY_SIZE - 1] = '\0';
	
	msgBuf[sizeof (msgBuf) - 1] = '\0';
	msgNoThread[sizeof (msgNoThread) - 1] = '\0';

	init_network();

	// install exit handlers
	atexit (exitCallback);
}

int
log_exit (int code)
{
	showBox = false;
	net_close(socket);

	return code;
}

void
log_setLevel (int level)
{
	maxLevel = level;
	//maxStreamLevel = level;
}

FILE *
log_setOutput (FILE *out)
{
	FILE *old = streamOut;
	streamOut = out;
	
	return old;
}

void
log_addV (log_Level level, const char *fmt, va_list list)
{
	log_Entry full_msg;
	vsnprintf (full_msg, sizeof (full_msg) - 1, fmt, list);
	int len = strlen(full_msg);
	full_msg[len] = '\n';
	full_msg[len+1] = '\0';
	full_msg[sizeof (full_msg) - 1] = '\0';
	
	if ((int)level <= maxStreamLevel)
	{
		fprintf (streamOut, "%s", full_msg);
		net_send(socket, full_msg, len+1, 0);
	}

	/*
	if ((int)level <= maxLevel)
	{
		int slot;

		queueNonThreaded ();
		
		slot = acquireSlot ();
		memcpy (queue[slot], full_msg, sizeof (queue[0]));
	}
	*/
}

void
log_add (log_Level level, const char *fmt, ...)
{
	va_list list;

	va_start (list, fmt);
	log_addV (level, fmt, list);
	va_end (list);
}

// non-threaded version of 'add'
// uses single-instance static storage with entry into the
// queue delayed until the next threaded 'add' or 'exit'
void
log_add_nothreadV (log_Level level, const char *fmt, va_list list)
{
	log_Entry full_msg;
	vsnprintf (full_msg, sizeof (full_msg) - 1, fmt, list);
	full_msg[sizeof (full_msg) - 1] = '\0';
	
	if ((int)level <= maxStreamLevel)
	{
		fprintf (streamOut, "%s\n", full_msg);
	}

	if ((int)level <= maxLevel)
	{
		memcpy (msgNoThread, full_msg, sizeof (msgNoThread));
		noThreadReady = true;
	}
}

void
log_add_nothread (log_Level level, const char *fmt, ...)
{
	va_list list;

	va_start (list, fmt);
	log_add_nothreadV (level, fmt, list);
	va_end (list);
}

void
log_showBox (bool show, bool err)
{
	showBox = show;
	errorBox = err;
}

// sets the maximum log lines captured for the final
// display to the user on failure exit
void
log_captureLines (int num)
{
	if (num > MAX_LOG_ENTRIES)
		num = MAX_LOG_ENTRIES;
	if (num < 1)
		num = 1;
	maxDisp = num;

	// remove any extra lines already on queue
	removeExcess (0);
}

static void
exitCallback (void)
{
	if (showBox)
		displayLog (errorBox);

	log_exit (0);
}

static void
displayLog (bool isError)
{
	char *p = msgBuf;
	int left = sizeof (msgBuf) - 1;
	int len;
	int ptr;

	if (isError)
	{
		strcpy (p, "The Ur-Quan Masters encountered a fatal error.\n"
				"Part of the log follows:\n\n");
		len = strlen (p);
		p += len;
		left -= len;
	}

	// Glue the log entries together
	// Locking is not a good idea at this point and we do not
	// really need it -- the worst that can happen is we get
	// an extra or an incomplete message
	for (ptr = qtail; ptr != qhead && left > 0;
			ptr = (ptr + 1) % MAX_LOG_ENTRIES)
	{
		len = strlen (queue[ptr]) + 1;
		if (len > left)
			len = left;
		memcpy (p, queue[ptr], len);
		p[len - 1] = '\n';
		p += len;
		left -= len;
	}

	// Glue the non-threaded message if present
	if (noThreadReady)
	{
		noThreadReady = false;
		len = strlen (msgNoThread);
		if (len > left)
			len = left;
		memcpy (p, msgNoThread, len);
		p += len;
		left -= len;
	}
	
	*p = '\0';

	log_displayBox ("The Ur-Quan Masters", isError, msgBuf);
}

