/*
 * dlsof.h - illumos header file for /proc-based lsof
 */


/*
 * Copyright 1997 Purdue Research Foundation, West Lafayette, Indiana
 * 47907.  All rights reserved.
 *
 * Written by Victor A. Abell
 *
 * This software is not subject to any license of the American Telephone
 * and Telegraph Company or the Regents of the University of California.
 *
 * Permission is granted to anyone to use this software for any purpose on
 * any computer system, and to alter it and redistribute it freely, subject
 * to the following restrictions:
 *
 * 1. Neither the authors nor Purdue University are responsible for any
 *    consequences of the use of this software.
 *
 * 2. The origin of this software must not be misrepresented, either by
 *    explicit claim or by omission.  Credit to the authors and Purdue
 *    University must appear in documentation and sources.
 *
 * 3. Altered versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 4. This notice may not be removed or altered.
 */


#if	!defined(ILLUMOS_LSOF_H)
#define	ILLUMOS_LSOF_H	1

/*
#include <fcntl.h>
#include <malloc.h>
*/
#include <dirent.h>
#include <limits.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rpc/rpc.h>
#include <rpc/clnt_soc.h>
#include <rpc/pmap_prot.h>
#include <rpc/rpcent.h>
#include <sys/sysmacros.h>

#include <procfs.h>
#include <libproc.h>

/*
 * This definition is needed for the common function prototype definitions
 * in "proto.h", but isn't used in /proc-based lsof.
 */

typedef	unsigned long	KA_T;

#define	COMP_P		const void
#define	READLEN_T	int
#define MALLOC_P	char
#define FREE_P		MALLOC_P
#define MALLOC_S	unsigned
#define DIRTYPE		dirent
#define QSORT_P		char
#define DEVINCR		1024

#if 0
#define GET_MAJ_DEV(d)	((major_t)(d >> L_BITSMINOR & L_MAXMAJ))
#define GET_MIN_DEV(d)	((minor_t)(d & L_MAXMIN))
#else
#define GET_MAJ_DEV(d)	major((d))
#define GET_MIN_DEV(d)	minor((d))
#endif

/*
 * Global storage definitions (including their structure definitions)
 */

struct l_vfs {
	KA_T addr;			/* kernel address */
	char *dir;			/* mounted directory */
	char *fsname;			/* file system name */
	dev_t dev;			/* device */

# if	defined(HASFSINO)
	INODETYPE fs_ino;		/* file system inode number */
# endif	/* defined(HASFSINO) */

# if	defined(HASMNTSTAT)
	unsigned char mnt_stat;		/* mount point stat() status:
					 *     0 = succeeded
					 *     1 = failed */
# endif	/* defined(HASMNTSTAT) */


# if	solaris>=80000
	nlink_t nlink;			/* directory link count */
	off_t size;			/* directory size */
# endif	/* solaris>=80000 */

	struct l_vfs *next;		/* forward link */
};
extern struct l_vfs *Lvfs;

struct mounts {
        char *dir;              	/* directory (mounted on) */
        char *fsname;           	/* file system
					 * (symbolic links unresolved) */
	char *fsnmres;			/* file system
					 * (symbolic links resolved) */
        dev_t dev;              	/* directory st_dev */
	dev_t rdev;			/* directory st_rdev */
	INODETYPE inode;		/* directory st_ino */
	mode_t mode;			/* directory st_mode */
	mode_t fs_mode;			/* file system st_mode */
        struct mounts *next;    	/* forward link */
	char *fstype;			/* file system type */
	nlink_t nlink;			/* directory st_nlink */
	off_t size;			/* directory st_size */
	unsigned char stat;		/* mount point stat() status:
					 *     0 = succeeded
					 *     1 = failed */
};

struct sfile {
	char *aname;			/* argument file name */
	char *name;			/* file name (after readlink()) */
	char *devnm;			/* device name (optional) */
	dev_t dev;			/* device */
	dev_t rdev;			/* raw device */
	mode_t mode;			/* S_IFMT mode bits from stat() */
	int type;			/* file type: 0 = file system
				 	 *	      1 = regular file */
	INODETYPE i;			/* inode number */
	int f;				/* file found flag */
	struct mounts *mp;		/* mount structure pointer for file
					 * system type entries */
#define	SAVE_MP_IN_SFILE	1	/* for ck_file_arg() im arg.c */
	struct sfile *next;		/* forward link */
};

#endif	/* ILLUMOS_LSOF_H	*/
