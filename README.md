CSCC69 Project 2
Summer 2013
=============

CSCC69 


<h2>Outline</h2>

<ol>
<li> Introduction</a> </li>
<li> Setup</a></li>
<li> Project Tasks</a></li>
<li> Paging Details</a></li>
<li> Paging in OS/161</a></li>

</ol>
<h2><a name="intro">Introduction</a></h2>

<p> You have improved OS/161 to the point that you can now run
multiple user processes (e.g. collecting exit status with waitpid, and
sending signals). However, there are a number of shortcomings in the
current system.  For example, the total size of all processes is
limited by the number of TLB entries (i.e., 64 pages). Your goal in
this project is to complete the implementation of virtual memory in
OS/161 to allow multiple processes to share limited physical memory
while providing each process with the abstraction of a very large
virtual memory. 
</p>

<p> Even with the paging details in this handout and in the design
document, we expect you will spend more time reading the new starter
code to understand the structure of the VM system provided than
actually writing the required function implementations. Pay particular
attention to the synchronization in the existing code. What does it
mean to acquire a page's lock? What does it mean to pin it?  To reduce
overhead, specialized lock implementations are often used instead of
the standard OS/161 lock interfaces. 
</p>


<h3>Overview of New Code</h3>
<p>The new and changed code for P2 includes:
</p><ul>
  <li>The code that manages the MIPS software-managed TLB (in <tt>kern/arch/mips/vm</tt>).</li>
  <li>Most of the code that implements paging -- the mechanism by which 
      memory pages of an active process can be sent to disk or restored to 
      memory (in <tt>kern/thread/vm</tt>).</li>
  <li>Updated header files for this functionality (in <tt>kern/include</tt> and <tt>kern/arch/mips/include</tt>).
</li></ul>
<p></p>

<p> Here is a brief summary of the new files:
</p><ul>
  <li> <tt>kern/vm/vmobj.c</tt> - code to manage "virtual memory objects", 
      which define regions of a process's virtual address space.</li>
  <li> <tt>kern/vm/lpage.c</tt> - code to manage "logical pages", the 
      structure that describes each page of the virtual adcdress space. <i>Most of the 
	  code you must write will be placed in this file.</i></li>
  <li> <tt>kern/vm/swap.c</tt> - code to manage the swap area, providing 
      functions to move pages between disk and main memory.</li>
  <li> <tt>kern/vm/addrspace.c</tt> - not new, but completely rewritten to set 
      up and manage a process's virtual address space using the new vmobj and 
      lpage structures.</li>
  <li> <tt>kern/arch/mips/vm/vm.c</tt> - machine dependent part of virtual 
      memory management; this is the starting point when a TLB miss exception 
      occurs.</li>
  <li> <tt>kern/arch/mips/vm/coremap.c</tt> - machine dependent part of 
      memory management responsible for the allocation / reclamation of 
      physical pages of memory. <i>The functions that select a page to be 
	  replaced should be implemented here.</i></li>
  <li> <tt>kern/test/coremaptest.c</tt> - test of the existing coremap 
      functions. <i>These are invoked as "cm" and "cm2" from the os161 menu prompt</i><p></p></li>
</ul>
<p>
To help you get started, base-design.txt describes 
the overall design of the virtual memory system that you will be completing. 
<b>You should read this file before starting to read the code.</b></p>

<h3><a name="setup">Setup</a></h3>

<p>In P1, you may have run into RAM size limits and edited your
<tt>sys161.conf</tt> to make more memory available. However, if you
try to boot your kernel with more memory than is compatible with your
swap disk, the kernel will refuse to boot. In fact, the
<tt>swap_bootstrap</tt> function checks to ensure that the swap disk
is at least 20X the size of RAM.
</p>
<p>There are two ways to deal with this. You can either restrict your
physical memory to 512 kB by setting the <tt>ramsize</tt> line in your
<tt>sys161.conf</tt> file, or you can use a larger
<tt>ramsize</tt>value and also increase the size of your swap disk in
your <tt>sys161.conf</tt> file. You may also want to test your kernel
with less than 512 kB so you run out memory and start paging
sooner. We will be testing with 512 kB or less.  The <tt>sys161.conf</tt> file
provided with the P2 starter code is configured with 512K of memory and a 10M
swap disk, <b>but you must remember to copy this <tt>sys161.conf</tt> file to
your <tt>~/cscc69/root</tt> directory, where you run the kernel.</b> 
</p>

<p>Now, config an ASST2-NORAND kernel. (ASST2-NORAND uses predictable
eviction operations, so it's easier to test. Use ASST2-RAND for
testing once you are confident in your implementation.) Build your
kernel -- and report any errors (if any) on the discussion board!. You
are now ready to begin project 2. Remember to do all your work in
the <tt>P2</tt> folder in your repository.
</p>

<p> You will notice that <b>none</b> of the user-level programs can be
run now.  That is because attempting to load the program code segment
off disk into the process address space (see <tt>load_segment</tt> in
<tt>kern/syscall/loadelf.c</tt>) causes a page fault, which is not
currently handled. You may find it useful to set a breakpoint at
<tt>vm_fault</tt> (the starting point for handling all TLB misses) and step
through the execution until it returns EUNIMP.</p>

<h2><a name="tasks">Project Tasks</a></h2>

<p>Your mission is to implement missing functions in <tt>kern/vm/lpage.c</tt>
and <tt>kern/arch/mips/vm/coremap.c</tt> that handle page faults:</p>
<ul>
  <li><tt>lpage_fault</tt> --- handles a page fault. (Until you implement this
  function you won't be able to run any test user programs).</li>
  <li><tt>lpage_evict</tt> --- evicts an lpage from physical memory.</li>
  <li><tt>page_replace</tt> --- implements page replacement policy. You will
  write two versions of this function.</li>
</ul>

<h3>page_replace</h3>

<p>You will write both a sequential page replacement algorithm and a
random replacement algorithm. The sequential replacement algorithm
should keep track of the last page evicted from the coremap so that
the next entry in the coremap can be evicted in turn. Use the
ASST2-RAND configuration file to build a version of the kernel that
uses the random replacement algorithm and the ASST2-NORAND
configuration file to build the version that uses sequential
eviction.</p>

<p>You should code and debug your implementation of sequential page
replacement before using random eviction to test your code more
thoroughly. You do not need to worry about giving preference to clean
pages over dirty ones in either page replacement algorithm.</p>

<h2><a name="pg">Paging Details</a></h2>

<p> As covered in the lecture notes, the kernel creates the illusion
of unlimited memory by using physical memory as a cache of <b>virtual
pages</b>.  Paging relaxes the requirement that all the pages in a
process's virtual address space be in physical memory. Instead, we
allow a process to have pages either on disk or in memory. When a
process tries to access a page that is on disk (and not in physical
memory), a <b>page fault</b> occurs. The kernel must retrieve the page
from disk and load it into memory. Pages with valid TLB entries are
located in physical memory. This means that a reference to a page on
disk (and not in memory) will always generate a TLB fault. At the time
of a TLB fault, the hardware generates a TLB exception, trapping to
the kernel. The operating system then checks its page table to locate
the virtual page requested. If that page is currently in memory but
wasn't mapped by the TLB, then the kernel need only update the
TLB. However, if the page is on disk, the kernel must: </p>

<ol>
	<li> Allocate a frame in physical memory to store the page; 
	</li>
	<li> Read the page from disk; 
	</li>
	<li> Update the page table entry with the new virtual-to-physical 
	     address translation; 
	</li> 
	<li> Update the TLB to contain the new translation; and 
	</li> 
	<li> Resume execution of the user program (at least put it on 
	     the ready queue).<i>In os161, this is handled simply 
	     by returning from the TLB exception.</i>
	</li>
</ol>

<p> Notice that when the operating system selects a location in
physical memory in which to place the new page, that location may
already be occupied by another page. In this case, the operating
system must <b>evict</b> that other page from memory. If the page has
been modified or is not present in the swap area on disk, then the
page being evicted must be written to disk before the physical page
can be reallocated to the new page (the one that generated the page
fault above). If the page being evicted has not been modified and is
already present in the swap area on disk, then the kernel can avoid
writing the page to disk, but the appropriate page table entry still
must be updated to reflect the fact that the evicted page is no longer
in memory. </p>

<p> The first access to a part of the virtual address space that is
not loaded from the executable file (e.g. the first access to a page
in the stack or the heap) is treated as a special case. These pages
are not present in memory, but they have no representation on disk
either. In this case, the OS simply allocates a physical page and
zeros the contents before updating the page table and the TLB.<i>This
is often referred to as a "zero fill" page</i>.</p>

<p>As with any caching system, performance of your virtual memory
system depends on the policy used to decide which things are kept in
memory and which are evicted. On a page fault, the kernel must decide
which page to replace.  Ideally, it will evict a page that will not be
needed soon. Many operating systems (such as UNIX) avoid the delay of
synchronously writing memory pages to disk on a page fault by writing
modified pages to disk in advance, so that subsequent page faults can
be completed more quickly. (You do not need to implement this
optimization in this project.)</p>

<h2><a name="reqs">Paging in OS/161</a></h2>

<p> To complete the virtual memory system, you will write the function
that handles page faults (lpage_fault) and some of the functions it
calls. When you have completed this problem, your system will be able
to handle the exception that is generated when a process tries to
access an address that is not memory-resident. After handling the
exception, the system can continue running the user process. </p>

<p>You do not need to implement functions to move a page from disk to
memory and from memory to disk (see <tt>kern/vm/swap.c</tt>). However,
you should make sure you understand how the <b>backing store</b> is
implemented. (The backing store is the place on disk where you store
virtual pages not currently stored in physical memory - i.e. swap
space). How are evicted pages stored and how are they found when
needed?</p>

<p> When the time comes to bring a page into memory, you will need to
know which physical pages are currently in use. One way to manage
physical memory is to maintain a <b>core map</b>, a sort of reverse
page table. Instead of being indexed by virtual addresses, a core map
is indexed by its physical page number and contains the virtual
address and address space identifier for the virtual page currently
backed by the page in physical memory. When you need to evict a page,
you look up the physical address in the core map, locate the address
space whose page you are evicting and modify the corresponding state
information to indicate that the page will no longer be in
memory. Then you can evict the page. If the page is dirty, it must
first be written to the backing store. </p>

<p> The file <tt>kern/arch/mips/include/coremap.h</tt> defines the coremap
data structure which tracks the use of physical memory pages (implemented in
<tt>kern/arch/mips/vm/coremap.c</tt>).

</p><p>Your paging system must support page allocation requests generated
by <tt>kmalloc()</tt>, as well as from user programs. This code is
already implemented for you. However, <b>remember that kernel pages
cannot be paged out</b>. <i>Why not?</i></p>

<p>Although we are discarding the dumbvm implementation for this project,
you may find it worth looking at its code, especially <tt>vm_fault()</tt>.
Although it is simple, it should give you some useful insight into how other
parts of the code work. </p>

<p>Testing will be difficult in this project. You will need to
create a number of user programs that create large arrays of memory
and access them in predictable ways, so you can evaluate whether pages
are being paged in and out appropriately.  Some of the existing test programs
in <tt>src/user/testbin/</tt> can be used for this.  In particular, you should
try: <tt>huge</tt>, <tt>matmult</tt>, <tt>palin</tt>, and <tt>sort</tt>.
To test synchronization, you need to be running multiple processes that all use memory.  The existing <tt>triple*</tt> test programs do this, but depend on 
the <tt>execv</tt> system call, which is not implemented.  You should be able to create similar tests that just fork several child processes, each of which executes the matmult or sort function separately, however. </p>

<p>Good luck!</p>
