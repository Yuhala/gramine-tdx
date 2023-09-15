# Features

- Support for QEMU's `q35` (PCI transport) machine type

- Amount of memory that can be supplied by QEMU is restricted to 512GB

- Ring-0 code consists of:
  - BIOS:    at 4GB
  - PAL:     at 1.5M (and before 4MB)
  - LibOS:   close to end-of-RAM, but not in below regions
  - PTs:     at [146MB, 256MB)
  - VQs:     at [256MB, 512MB) -- this is "shared memory"
  - Hole:    from 2GB to 3GB (QEMU/KVM creates this hole)
  - PCI bus: from 3GB to 4GB
  - (the rest space is for app usage)

- Ring-3 / ring-0 separation

- GS segment register is used by Gramine kernel, assumed to never be used by
  user app (FS segment register is used by user app)

- Paging: flat single shared address space, no switching, all pages always
  present and RWX, 4KB pages (no huge tables)

- Time source: RDTSC (Invariant TSC) for relative time; absolute time is taken
  from the host on QEMU startup

- Timer interrupts: using TSC deadline mode, fired every 100us

- Timeouts, alarms, waiting/sleeping with timeouts

- Scheduling:
  - round robin, no fairness, no time slices
  - preemptive in ring-3 (upon timer interrupt)
  - cooperative (non-preemptive) in ring-0 (upon `_PalThreadYieldExecution` and
    blocking syscalls)

- XSAVE area: always saved and restored on context switches and interrupts

- Randomness: via `rdrand` instruction

- CPUID: mostly via `cpuid` instruction, but some returns are hard-coded

- No pre-defined environment variables

- Pipes: 4K buffer, blocking via `sched_thread_wait`/`sched_thread_wakeup`

- Console (stdin, stdout): uses virtio-console driver
  - stdin supports only non-interactive mode (input is assumed to be supplied
    from e.g. a file)

- Files: shared root directory, uses virtio-fs driver
  - on the host side, Gramine starts `virtiofsd --shared-dir /`

- Networking: uses virtio-vsock driver
  - may need to load the Linux kernel module: `sudo modprobe vhost_vsock`

# Not yet implemented

- `_PalThreadResume()`

- Multi-core (currently very single-core specific; data races are possible)

- CPU/NUMA topology (currently hard-coded single NUMA node and single core)

- Debugging support

- Eventfd

- Multi-processing