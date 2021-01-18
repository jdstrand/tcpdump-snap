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
static int (*original_chown) (const char *, const uid_t, const gid_t);

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
	} /* else {
		fprintf(stderr, "cannot setgid(%d), using %d\n", gid, g);
	} */
	return original_setgid(g);
}

int setuid(uid_t uid) {
	// lookup the libc's setuid() if we haven't already
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
	} /* else {
		fprintf(stderr, "cannot setuid(%d), using %d\n", uid, u);
	} */
	return original_setuid(u);
}

/*
 * tcpdump -w <file> causes tcpdump to do a chown(path, "tcpdump", "tcpdump")
 * which causes a DAC_OVERRIDE denial. So we instead set the capture directory
 * to 0770 with root:snap_daemon and then set these files as root:snap_daemon,
 * which allows the initial write but also -w to overwrite the same file. When
 * using -Z root, we see chown on root:root, so honor that.
 *
 * For now, this works ok with 0770 root:snap_daemon capture directory, but
 * the files are 0644. We might want to set the umask to 0002 instead so the
 * files of 0664.
 */
int chown(const char *pathname, uid_t uid, gid_t gid) {
	// lookup the libc's chown() if we haven't already
	if (!original_chown) {
		dlerror();
		original_chown = dlsym(RTLD_NEXT, "chown");
		if (!original_chown) {
			fprintf(stderr, "could not find chown in libc");
			return -1;
		}
		dlerror();
	}

	// we could look this up and fail, but snapd hardcodes it
	uid_t u = 0;
	gid_t g = 584788;
	if (uid == 0 && gid == 0) {
		u = 0;
		g = 0;
	}
	return original_chown(pathname, u, g);
}
