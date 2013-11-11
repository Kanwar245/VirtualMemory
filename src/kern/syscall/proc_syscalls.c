/*
 * Process-related syscalls.
 * New for A2.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/wait.h>
#include <lib.h>
#include <thread.h>
#include <current.h>
#include <pid.h>
#include <machine/trapframe.h>
#include <syscall.h>

/*
 * sys_fork
 * 
 * create a new process, which begins executing in md_forkentry().
 */


int
sys_fork(struct trapframe *tf, pid_t *retval)
{
	struct trapframe *ntf; /* new trapframe, copy of tf */
	int result;

	/*
	 * Copy the trapframe to the heap, because we might return to
	 * userlevel and make another syscall (changing the trapframe)
	 * before the child runs. The child will free the copy.
	 */

	ntf = kmalloc(sizeof(struct trapframe));
	if (ntf==NULL) {
		return ENOMEM;
	}
	*ntf = *tf; /* copy the trapframe */

	result = thread_fork(curthread->t_name, enter_forked_process, 
			     ntf, 0, retval);
	if (result) {
		kfree(ntf);
		return result;
	}

	return 0;
}

/*
 * sys_getpid
 * Placeholder to remind you to implement this.
 */
int
sys_getpid(pid_t *retval)
{
	*retval = curthread->t_pid;
	return 0;
}

/*
 * sys_waitpid
 * Placeholder comment to remind you to implement this.
 */
int
sys_waitpid(pid_t pid, int *status, int options, pid_t *retval)
{
    // The options argument should be 0 or WNOHANG
    if (options != 0 && options != WNOHANG) {
        DEBUG(DB_SYSCALL, "waitpid invalid option: %d\n", options);
        return EINVAL; // options argument requested invalid or unsupported options
    }
    
    // the pid argument named a nonexistent process
    if (pid == INVALID_PID) {
        return ESRCH; 
    }
    
    // status argument was an invalid pointer
    if (status == NULL) {
        return EFAULT; 
    }
    
    int result = pid_join(pid, status, options);
    if (result < 0) { // fail
        DEBUG (DB_SYSCALL, "thread_join failure: %d\n", result);
        *retval = -result;
        return -1;
    } else { // success
	 *retval = pid;
	 return 0;
    }
}

/*
 * sys_kill
 * Placeholder comment to remind you to implement this.
 */



