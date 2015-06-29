2400 #include "types.h"
2401 #include "defs.h"
2402 #include "param.h"
2403 #include "memlayout.h"
2404 #include "mmu.h"
2405 #include "x86.h"
2406 #include "proc.h"
2407 #include "spinlock.h"
2408 
2409 struct {
2410 	struct spinlock lock;
2411 	struct proc proc[NPROC];
2412 } ptable;
2413 
2414 static struct proc *initproc;
2415 
2416 int nextpid = 1;
2417 extern void forkret(void);
2418 extern void trapret(void);
2419 
2420 static void wakeup1(void *chan);
2421 
2422 void pinit(void) {
2423   initlock(&ptable.lock, "ptable");
2424 }
2425 
2426 
2427 
2428 
2429 
2430 
2431 
2432 
2433 
2434 
2435 
2436 
2437 
2438 
2439 
2440 
2441 
2442 
2443 
2444 
2445 
2446 
2447 
2448 
2449 
2450 // Look in the process table for an UNUSED proc.
2451 // If found, change state to EMBRYO and initialize
2452 // state required to run in the kernel.
2453 // Otherwise return 0.
2454 static struct proc* allocproc(int tickets) {
2455 	struct proc *p;
2456 	char *sp;
2457 
2458 	acquire(&ptable.lock);
2459 	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
2460 		if(p->state == UNUSED)
2461 			goto found;
2462 	release(&ptable.lock);
2463 	return 0;
2464 
2465 found:
2466 	p->state = EMBRYO;
2467 	p->stride = 0;
2468 	if(tickets <= 0) tickets = DEF_TICKETS;
2469 	p->step = CONSTANT/tickets;
2470 	p->pid = nextpid++;
2471 	release(&ptable.lock);
2472 
2473 	// Allocate kernel stack.
2474 	if((p->kstack = kalloc()) == 0){
2475 		p->state = UNUSED;
2476 		return 0;
2477 	}
2478 	sp = p->kstack + KSTACKSIZE;
2479 
2480 	// Leave room for trap frame.
2481 	sp -= sizeof *p->tf;
2482 	p->tf = (struct trapframe*)sp;
2483 
2484 	// Set up new context to start executing at forkret,
2485 	// which returns to trapret.
2486 	sp -= 4;
2487 	*(uint*)sp = (uint)trapret;
2488 
2489 	sp -= sizeof *p->context;
2490 	p->context = (struct context*)sp;
2491 	memset(p->context, 0, sizeof *p->context);
2492 	p->context->eip = (uint)forkret;
2493 
2494 	return p;
2495 }
2496 
2497 
2498 
2499 
2500 // Set up first user process.
2501 void userinit(void) {
2502 	struct proc *p;
2503 	extern char _binary_initcode_start[], _binary_initcode_size[];
2504 
2505 	p = allocproc(DEF_TICKETS);
2506 	initproc = p;
2507 	if((p->pgdir = setupkvm()) == 0)
2508 		panic("userinit: out of memory?");
2509 	inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
2510 	p->sz = PGSIZE;
2511 	memset(p->tf, 0, sizeof(*p->tf));
2512 	p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
2513 	p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
2514 	p->tf->es = p->tf->ds;
2515 	p->tf->ss = p->tf->ds;
2516 	p->tf->eflags = FL_IF;
2517 	p->tf->esp = PGSIZE;
2518 	p->tf->eip = 0;  // beginning of initcode.S
2519 
2520 	safestrcpy(p->name, "initcode", sizeof(p->name));
2521 	p->cwd = namei("/");
2522 
2523 	p->state = RUNNABLE;
2524 }
2525 
2526 // Grow current process's memory by n bytes.
2527 // Return 0 on success, -1 on failure.
2528 int growproc(int n) {
2529 	uint sz;
2530 
2531 	sz = proc->sz;
2532 	if(n > 0) {
2533 		if((sz = allocuvm(proc->pgdir, sz, sz + n)) == 0)
2534 		return -1;
2535 	} else if(n < 0) {
2536 		if((sz = deallocuvm(proc->pgdir, sz, sz + n)) == 0)
2537 		return -1;
2538 	}
2539 	proc->sz = sz;
2540 	switchuvm(proc);
2541 	return 0;
2542 }
2543 
2544 
2545 
2546 
2547 
2548 
2549 
2550 // Create a new process copying p as the parent.
2551 // Sets up stack to return as if from system call.
2552 // Caller must set state of returned proc to RUNNABLE.
2553 int fork(int tickets) {
2554 	int i, pid;
2555 	struct proc *np;
2556 
2557 	// Allocate process.
2558 	if((np = allocproc(tickets)) == 0)
2559 		return -1;
2560 
2561 	// Copy process state from p.
2562 	if((np->pgdir = copyuvm(proc->pgdir, proc->sz)) == 0){
2563 		kfree(np->kstack);
2564 		np->kstack = 0;
2565 		np->state = UNUSED;
2566 		return -1;
2567 	}
2568 	np->sz = proc->sz;
2569 	np->parent = proc;
2570 	*np->tf = *proc->tf;
2571 
2572 	// Clear %eax so that fork returns 0 in the child.
2573 	np->tf->eax = 0;
2574 
2575 	for(i = 0; i < NOFILE; i++)
2576 		if(proc->ofile[i])
2577 			np->ofile[i] = filedup(proc->ofile[i]);
2578 	np->cwd = idup(proc->cwd);
2579 
2580 	safestrcpy(np->name, proc->name, sizeof(proc->name));
2581 
2582 	pid = np->pid;
2583 
2584 	// lock to force the compiler to emit the np->state write last.
2585 	acquire(&ptable.lock);
2586 	np->state = RUNNABLE;
2587 	release(&ptable.lock);
2588 
2589 	return pid;
2590 }
2591 
2592 
2593 
2594 
2595 
2596 
2597 
2598 
2599 
2600 // Exit the current process.  Does not return.
2601 // An exited process remains in the zombie state
2602 // until its parent calls wait() to find out it exited.
2603 void exit(void) {
2604 	struct proc *p;
2605 	int fd;
2606 
2607 	if(proc == initproc)
2608 		panic("init exiting");
2609 
2610 	// Close all open files.
2611 	for(fd = 0; fd < NOFILE; fd++){
2612 		if(proc->ofile[fd]){
2613 			fileclose(proc->ofile[fd]);
2614 			proc->ofile[fd] = 0;
2615 		}
2616 	}
2617 
2618 	begin_op();
2619 	iput(proc->cwd);
2620 	end_op();
2621 	proc->cwd = 0;
2622 
2623 	acquire(&ptable.lock);
2624 
2625 	// Parent might be sleeping in wait().
2626 	wakeup1(proc->parent);
2627 
2628 	// Pass abandoned children to init.
2629 	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2630 		if(p->parent == proc){
2631 			p->parent = initproc;
2632 			if(p->state == ZOMBIE)
2633 				wakeup1(initproc);
2634 		}
2635 	}
2636 
2637 	// Jump into the scheduler, never to return.
2638 	proc->state = ZOMBIE;
2639 	sched();
2640 	panic("zombie exit");
2641 }
2642 
2643 
2644 
2645 
2646 
2647 
2648 
2649 
2650 // Wait for a child process to exit and return its pid.
2651 // Return -1 if this process has no children.
2652 int wait(void) {
2653 	struct proc *p;
2654 	int havekids, pid;
2655 
2656 	acquire(&ptable.lock);
2657 	for(;;) {
2658 		// Scan through table looking for zombie children.
2659 		havekids = 0;
2660 		for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2661 			if(p->parent != proc)
2662 				continue;
2663 			havekids = 1;
2664 			if(p->state == ZOMBIE){
2665 
2666 				cprintf("process: %d | processname: %s\n", p->pid, p->name);
2667 
2668 				// Found one.
2669 				pid = p->pid;
2670 				kfree(p->kstack);
2671 				p->kstack = 0;
2672 				freevm(p->pgdir);
2673 				p->state = UNUSED;
2674 				p->pid = 0;
2675 				p->parent = 0;
2676 				p->name[0] = 0;
2677 				p->killed = 0;
2678 				release(&ptable.lock);
2679 				return pid;
2680 			}
2681 		}
2682 
2683 		// No point waiting if we don't have any children.
2684 		if(!havekids || proc->killed){
2685 			release(&ptable.lock);
2686       		return -1;
2687     	}
2688 
2689 		// Wait for children to exit.  (See wakeup1 call in proc_exit.)
2690 		sleep(proc, &ptable.lock);  //DOC: wait-sleep
2691 	}
2692 }
2693 
2694 
2695 
2696 
2697 
2698 
2699 
2700 // Per-CPU process scheduler.
2701 // Each CPU calls scheduler() after setting itself up.
2702 // Scheduler never returns.  It loops, doing:
2703 //  - choose a process to run
2704 //  - swtch to start running that process
2705 //  - eventually that process transfers control
2706 //      via swtch back to the scheduler.
2707 void scheduler(void) {
2708 	int stride;
2709 	struct proc *p, *m;
2710 
2711 	while(1) {
2712 		// Enable interrupts on this processor.
2713 		sti();
2714 
2715 		// Loop over process table looking for process to run.
2716 		acquire(&ptable.lock);
2717 		stride = MAX_STRIDE;
2718 		p = 0;
2719 		for(m = ptable.proc; m < &ptable.proc[NPROC]; m++) {
2720 			if((m->state == RUNNABLE) && (m->stride < stride)) {
2721 				stride = m->stride;
2722 				p = m;
2723 				cprintf("passo: %d, passada: %d\n", m->step, m->stride);
2724 			}
2725 		}
2726 
2727 		// Switch to chosen process.  It is the process's job
2728 		// to release ptable.lock and then reacquire it
2729 		// before jumping back to us.
2730 
2731 		if(p){
2732 
2733 			p->stride += p->step;
2734 			proc = p;
2735 			switchuvm(p);
2736 			p->state = RUNNING;
2737 			swtch(&cpu->scheduler, proc->context);
2738 			switchkvm();
2739 
2740 			// Process is done running for now.
2741 			// It should have changed its p->state before coming back.
2742 			proc = 0;
2743 		}
2744 
2745 		release(&ptable.lock);
2746 	}
2747 }
2748 
2749 
2750 // Enter scheduler.  Must hold only ptable.lock
2751 // and have changed proc->state.
2752 void sched(void) {
2753 	int intena;
2754 
2755 	if(!holding(&ptable.lock))
2756 		panic("sched ptable.lock");
2757 	if(cpu->ncli != 1)
2758 		panic("sched locks");
2759 	if(proc->state == RUNNING)
2760 		panic("sched running");
2761 	if(readeflags()&FL_IF)
2762 		panic("sched interruptible");
2763 	intena = cpu->intena;
2764 	swtch(&proc->context, cpu->scheduler);
2765 	cpu->intena = intena;
2766 }
2767 
2768 // Give up the CPU for one scheduling round.
2769 void yield(void) {
2770 	acquire(&ptable.lock);  //DOC: yieldlock
2771 	proc->state = RUNNABLE;
2772 	sched();
2773 	release(&ptable.lock);
2774 }
2775 
2776 // A fork child's very first scheduling by scheduler()
2777 // will swtch here.  "Return" to user space.
2778 void forkret(void) {
2779 	static int first = 1;
2780 	// Still holding ptable.lock from scheduler.
2781 	release(&ptable.lock);
2782 
2783 	if (first) {
2784 		// Some initialization functions must be run in the context
2785 		// of a regular process (e.g., they call sleep), and thus cannot
2786 		// be run from main().
2787 		first = 0;
2788 		initlog();
2789 	}
2790 
2791   // Return to "caller", actually trapret (see allocproc).
2792 }
2793 
2794 
2795 
2796 
2797 
2798 
2799 
2800 // Atomically release lock and sleep on chan.
2801 // Reacquires lock when awakened.
2802 void sleep(void *chan, struct spinlock *lk) {
2803 	if(proc == 0)
2804  		panic("sleep");
2805 
2806 	if(lk == 0)
2807 		panic("sleep without lk");
2808 
2809 	// Must acquire ptable.lock in order to
2810 	// change p->state and then call sched.
2811 	// Once we hold ptable.lock, we can be
2812 	// guaranteed that we won't miss any wakeup
2813 	// (wakeup runs with ptable.lock locked),
2814 	// so it's okay to release lk.
2815 	if(lk != &ptable.lock){  //DOC: sleeplock0
2816 		acquire(&ptable.lock);  //DOC: sleeplock1
2817 		release(lk);
2818 	}
2819 
2820 	// Go to sleep.
2821 	proc->chan = chan;
2822 	proc->state = SLEEPING;
2823 	sched();
2824 
2825 	// Tidy up.
2826 	proc->chan = 0;
2827 
2828 	// Reacquire original lock.
2829 	if(lk != &ptable.lock){  //DOC: sleeplock2
2830 		release(&ptable.lock);
2831 		acquire(lk);
2832 	}
2833 }
2834 
2835 
2836 
2837 
2838 
2839 
2840 
2841 
2842 
2843 
2844 
2845 
2846 
2847 
2848 
2849 
2850 // Wake up all processes sleeping on chan.
2851 // The ptable lock must be held.
2852 static void wakeup1(void *chan) {
2853 	struct proc *p;
2854 
2855 	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
2856 		if(p->state == SLEEPING && p->chan == chan)
2857 	p->state = RUNNABLE;
2858 }
2859 
2860 // Wake up all processes sleeping on chan.
2861 void wakeup(void *chan) {
2862 	acquire(&ptable.lock);
2863 	wakeup1(chan);
2864 	release(&ptable.lock);
2865 }
2866 
2867 // Kill the process with the given pid.
2868 // Process won't exit until it returns
2869 // to user space (see trap in trap.c).
2870 int kill(int pid) {
2871 	struct proc *p;
2872 
2873 	acquire(&ptable.lock);
2874 	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2875 		if(p->pid == pid){
2876 			p->killed = 1;
2877 			// Wake process from sleep if necessary.
2878 			if(p->state == SLEEPING)
2879  				p->state = RUNNABLE;
2880 			release(&ptable.lock);
2881 			return 0;
2882 		}
2883 	}
2884 	release(&ptable.lock);
2885 	return -1;
2886 }
2887 
2888 
2889 
2890 
2891 
2892 
2893 
2894 
2895 
2896 
2897 
2898 
2899 
2900 // Print a process listing to console.  For debugging.
2901 // Runs when user types ^P on console.
2902 // No lock to avoid wedging a stuck machine further.
2903 void procdump(void) {
2904 	static char *states[] = {
2905 		[UNUSED]    "unused",
2906 		[EMBRYO]    "embryo",
2907 		[SLEEPING]  "sleep ",
2908 		[RUNNABLE]  "runble",
2909 		[RUNNING]   "run   ",
2910 		[ZOMBIE]    "zombie"
2911 	};
2912 	int i;
2913 	struct proc *p;
2914 	char *state;
2915 	uint pc[10];
2916 
2917 	for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2918 		if(p->state == UNUSED)
2919  			continue;
2920 		if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
2921  			state = states[p->state];
2922 		else
2923 			state = "???";
2924 		cprintf("%d %s %s", p->pid, state, p->name);
2925 		if(p->state == SLEEPING){
2926 			getcallerpcs((uint*)p->context->ebp+2, pc);
2927 			for(i=0; i<10 && pc[i] != 0; i++)
2928 				cprintf(" %p", pc[i]);
2929 		}
2930 		cprintf("\n");
2931 	}
2932 }
2933 
2934 
2935 
2936 
2937 
2938 
2939 
2940 
2941 
2942 
2943 
2944 
2945 
2946 
2947 
2948 
2949 
