/*
 * dproc.c - illumos process access functions for /proc-based lsof
 */

#include "lsof.h"

static int
foreach_file(const prfdinfo_t *pr, void *psinfop)
{
	psinfo_t *psinfo = psinfop;

	printf("    FD %u\n", pr->pr_fd);

	return 0;
}

static int
foreach_process(psinfo_t *ps, lwpsinfo_t *lwp, void *arg __unused)
{
	struct ps_prochandle *P;
	static pid_t me = -1;
	char cmd[MAXPATHLEN + 1], pbuf[MAXPATHLEN + 1], nmabuf[MAXPATHLEN + 1];
	short pss, sf;
	int err;

	if (me == -1)
		me = getpid();

	if (ps->pr_pid == me)
		return 0;

	P = Pgrab(ps->pr_pid, PGRAB_NOSTOP, &err);
	if (P == NULL)
		return 0;

	if (Pexecname(P, cmd, sizeof (cmd)) == NULL)
	{
		Prelease(P, PRELEASE_CLEAR);
		return 0;
	}

	printf("PID %u (%s)\n", ps->pr_pid, cmd);

	if (is_proc_excl(ps->pr_pid, ps->pr_pgid, ps->pr_uid, &pss, &sf) ||
	    is_cmd_excl(cmd, &pss, &sf)) {
		return 1;
	}

	alloc_lproc(ps->pr_pid, ps->pr_pgid, ps->pr_ppid, ps->pr_uid, cmd,
	    (int)pss, (int)sf);

	/* Task information */

	/* CWD */
	snpf(pbuf, sizeof (pbuf), "/proc/%u/cwd", ps->pr_pid);
	if ((ret = readlink(pbuf, nmabuf, sizeof (nmabuf))) > 0) {
		struct stat st;
		int ss;

		nmabuf[ret] = '\0';
		alloc_lfile(CWD, -1);
		process_proc_node(lpath, path, &st, ss,
		    (struct stat *)NULL, 0);
		if (Lf->sf)
			link_lfile();
	}

	return (proc_fdwalk(ps->pr_pid, foreach_file, ps));
}

void
gather_proc_info()
{
	/*
	 * Read /proc, looking for PID directories.  Open each one and
	 * gather its process and file information.
	 */
	(void) proc_walk(foreach_process, NULL, PR_WALK_PROC);
}

/*
 * initialize() - perform all initialisation
 */

void
initialize()
{
/*
 * Test for -i and -X option conflict.
 */
	if (Fxopt && (Fnet || Nwad)) {
	    (void) fprintf(stderr, "%s: -i is useless when -X is specified.\n",
		Pn);
	    usage(1, 0, 0);
	}
}

static int
hash_zn(char *zn)
{
	register int i, h;
	size_t l;

	if (!(l = strlen(zn)))
	    return(0);
	if (l == 1)
	    return((int)*zn & (HASHZONE - 1));
	for (i = h = 0; i < (int)(l - 1); i++) {
	    h ^= ((int)zn[i] * (int)zn[i+1]) << ((i*3)%13);
	}
	return(h & (HASHZONE - 1));
}

int
enter_zone_arg(char *zn)
{
	int zh;
	znhash_t *zp, *zpn;
	/*
	 * Allocate zone argument hash space, as required.
	 */
	if (!ZoneArg) {
	    if (!(ZoneArg = (znhash_t **)calloc(HASHZONE, sizeof(znhash_t *))))
	    {
		(void) fprintf(stderr, "%s: no space for zone arg hash\n", Pn);
		Exit(1);
	    }
	}
	/*
	 * Hash the zone name and search the argument hash.
	 */
	zh = hash_zn(zn);
	for (zp = ZoneArg[zh]; zp; zp = zp->next) {
	    if (!strcmp(zp->zn, zn))
		break;
	}
	if (zp)	{

	/*
	 * Process a duplicate.
	 */
	    if (!Fwarn)
		(void) fprintf(stderr, "%s: duplicate zone name: %s\n", Pn, zn);
	    return(1);
	}
	/*
	 * Create a new hash entry and link it to its bucket.
	 */
	if (!(zpn = (znhash_t *)malloc((MALLOC_S)sizeof(znhash_t)))) {
	    (void) fprintf(stderr, "%s no hash space for zone: %s\n", Pn, zn);
	    Exit(1);
	}
	zpn->f = 0;
	zpn->zn = zn;
	zpn->next = ZoneArg[zh];
	ZoneArg[zh] = zpn;
	return(0);
}

