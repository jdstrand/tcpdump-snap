#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/*
 * $ gcc -Wall -fPIC -shared -o mylib.so ./lib.c -ldl
 * $ LD_PRELOAD=./mylib.so ...
 */

#include <dlfcn.h>
#include <grp.h>
#include <stdio.h>
#include <unistd.h>

static int (*original_setgroups) (size_t, const gid_t[]);
static int (*original_initgroups) (const char *, const gid_t);
static int (*original_setgid) (const gid_t);
static int (*original_setuid) (const uid_t);

int setgroups(size_t size, const gid_t *list) {
	// lookup the libc's setgroups() if we haven't already
	if (!original_setgroups) {
		dlerror();
		original_setgroups = dlsym(RTLD_NEXT, "setgroups");
		if (!original_setgroups) {
			fprintf(stderr, "could not find setgroups in libc");
			return -1;
		}
		dlerror();
	}

	return original_setgroups(0, NULL);
}

int initgroups(const char *user, const gid_t group) {
	// lookup the libc's setgroups() if we haven't already
	if (!original_initgroups) {
		dlerror();
		original_initgroups = dlsym(RTLD_NEXT, "initgroups");
		if (!original_initgroups) {
			fprintf(stderr, "could not find initgroups in libc");
			return -1;
		}
		dlerror();
	}

	return setgroups(0, NULL);
}

/*
 * The tcpdump from the Ubuntu will setuid/setgid to the tcpdump user, which
 * isn't allowed by the sandbox policy. The -Z allows choosing another user,
 * so allow -Z to root, otherwise always drop to snap_daemon.
 */
int setgid(gid_t gid) {
	// lookup the libc's setgid() if we haven't already
	if (!original_setgid) {
		dlerror();
		original_setgid = dlsym(RTLD_NEXT, "setgid");
		if (!original_setgid) {
			fprintf(stderr, "could not find setgid in libc");
			return -1;
		}
		dlerror();
	}

	// we could look this up and fail, but snapd hardcodes it
	gid_t g = 584788;
	if (gid == 0) {
		g = gid;
	} else {
		fprintf(stderr, "cannot setgid(%d), using %d\n", gid, g);
	}
	return original_setgid(g);
}

int setuid(uid_t uid) {
	// lookup the libc's setgid() if we haven't already
	if (!original_setuid) {
		dlerror();
		original_setuid = dlsym(RTLD_NEXT, "setuid");
		if (!original_setuid) {
			fprintf(stderr, "could not find setuid in libc");
			return -1;
		}
		dlerror();
	}

	// we could look this up and fail, but snapd hardcodes it
	uid_t u = 584788;
	if (uid == 0) {
		u = uid;
	} else {
		fprintf(stderr, "cannot setuid(%d), using %d\n", uid, u);
	}
	return original_setuid(u);
}
