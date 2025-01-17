/* SPDX-License-Identifier: LGPL-3.0-or-later */
/* Copyright (C) 2023 Intel Corporation */

/*
 * Declarations for threads and their contexts (for switching).
 *
 * Also declares idle and bottomhalves thread loops.
 */

#pragma once

#include <stdint.h>

#include "api.h"
#include "list.h"

#include "kernel_sched.h"

#define THREAD_STACK_SIZE (PAGE_SIZE * 16) /* 64KB user stack */
#define ALT_STACK_SIZE    (PAGE_SIZE * 2)  /* 8KB signal stack */

enum thread_state {
    THREAD_STOPPED,
    THREAD_RUNNABLE,
    THREAD_BLOCKED,
    THREAD_RUNNING,
};

struct thread_context {
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rbp;
    uint64_t rbx;
    uint64_t rdx;
    uint64_t rax;
    uint64_t rcx;
    uint64_t rsp;
    uint64_t rip;
    uint64_t rflags;

	uint64_t user_rip;
	uint64_t user_fsbase;

    uint64_t csgsfsss;
    uint64_t err;
    uint64_t trapno;
    uint64_t oldmask;
    uint64_t cr2;

    void* fpregs; /* XSAVE area */
} __attribute__((packed));

struct thread_irq_pseudo_stack {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed));

DEFINE_LIST(thread);
struct thread {
    LIST_TYPE(thread) list;
    enum thread_state state;
    uint32_t thread_id; /* for debugging purposes */
    int* blocked_on;
    bool is_helper; /* is it an idle or bottomhalves thread */

    /* CPU affinity of a thread */
    unsigned long cpu_mask[MAX_NUM_CPU_LONGS];

    /* for context switching: GPRs + XSAVE area of a thread during kernel execution (when it
     * explicitly yields, i.e., we implement cooperative kernel scheduling) or during userland
     * execution (when it is interrupted by IRQ); XSAVE is always stored in a preallocated region */
    struct thread_context context;

    struct thread_irq_pseudo_stack irq_pseudo_stack;

    /* per-thread scratch registers for returning to userspace from rt_sigreturn() syscall;
     * sigreturn flow is different from normal-return-to-userspace sysret flow in that sigreturn
     * uses iretq instruction which requires RIP,RSP,RFLAGS to be located on a (pseudo) stack;
     * see also kernel_events.S */
    uint64_t sigreturn_user_rsp;
    uint64_t sigreturn_user_rax;
    uint64_t sigreturn_pseudo_rsp; /* &sigreturn_pseudo_stack + sizeof(thread_irq_pseudo_stack) */
    struct thread_irq_pseudo_stack sigreturn_pseudo_stack;
};
DEFINE_LISTP(thread);

int thread_get_stack_and_fpregs(void** out_stack, void** out_fpregs);
noreturn void thread_free_stack_and_die(void* thread_stack, int* clear_child_tid);

void thread_setup(struct thread* thread, void* fpregs, void* stack, int (*callback)(void*),
                  const void* param);
int thread_helper_create(int (*callback)(void*), struct thread** out_thread);

noreturn int thread_idle_run(void* args);
noreturn int thread_bottomhalves_run(void* args);
