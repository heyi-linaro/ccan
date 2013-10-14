/* Licensed under LGPLv2.1+ - see LICENSE file for details */
#ifndef CCAN_IO_PLAN_H
#define CCAN_IO_PLAN_H
struct io_conn;

/**
 * struct io_plan - a plan of what I/O to do.
 * @pollflag: POLLIN or POLLOUT.
 * @io: function to call when fd is available for @pollflag.
 * @next: function to call after @io returns true.
 * @next_arg: argument to @next.
 * @u: scratch area for I/O.
 *
 * When the fd is POLLIN or POLLOUT (according to @pollflag), @io is
 * called.  If it returns -1, io_close() becomed the new plan (and errno
 * is saved).  If it returns 1, @next is called, otherwise @io is
 * called again when @pollflag is available.
 *
 * You can use this to write your own io_plan functions.
 */
struct io_plan {
	int pollflag;
	/* Only NULL if idle. */
	int (*io)(int fd, struct io_plan *plan);
	/* Only NULL if closing. */
	struct io_plan (*next)(struct io_conn *, void *arg);
	void *next_arg;

	union {
		struct {
			char *buf;
			size_t len;
		} read;
		struct {
			const char *buf;
			size_t len;
		} write;
		struct {
			char *buf;
			size_t *lenp;
		} readpart;
		struct {
			const char *buf;
			size_t *lenp;
		} writepart;
		struct {
			int saved_errno;
		} close;
		struct {
			void *p;
			size_t len;
		} ptr_len;
		struct {
			void *p1;
			void *p2;
		} ptr_ptr;
		struct {
			size_t len1;
			size_t len2;
		} len_len;
	} u;
};

#ifdef DEBUG
/**
 * io_debug - routine to select connection(s) to debug.
 *
 * If this is set, the routine should return true if the connection is a
 * debugging candidate.  If so, the callchain for I/O operations on this
 * connection will be linear, for easier use of a debugger.
 */
extern bool (*io_debug)(struct io_conn *conn);

/**
 * io_plan_other - mark the next plan not being for the current connection
 *
 * Most routines which take a plan are about to apply it to the current
 * connection.  We (ab)use this pattern for debugging: as soon as such a
 * plan is created, it is called, to create a linear call chain.
 *
 * Some routines, like io_break() and io_wake() take an io_plan, but they
 * must not be applied immediately to the current connection, so we call this
 * first.
 */
#define io_plan_other() ((io_plan_for_other = true))

/**
 * io_plan_debug - hook for debugging a plan.
 *
 * After constructing a plan, call this.  If the current connection is being
 * debugged, then it will be immediately serviced with this plan.
 */
void io_plan_debug(struct io_plan *plan);
extern bool io_plan_for_other;
#else
#define io_plan_other() (void)0
static inline void io_plan_debug(struct io_plan *plan) { }
#endif

#endif /* CCAN_IO_PLAN_H */
