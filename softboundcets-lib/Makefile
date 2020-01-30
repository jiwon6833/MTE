all: softboundcets_rt softboundmpx_rt

MPX_FLAGS=-Wall -pedantic -O0 -D__SOFTBOUNDMPX_SPATIAL -g
CFLAGS=-Wall -pedantic  -D__SOFTBOUNDCETS_SPATIAL -O2 -fno-strict-aliasing -g # gykim spatial
RISCV_CFLAGS=-target riscv64-unknown-linux-gnu -Wall -pedantic  -D__SOFTBOUNDCETS_SPATIAL -O0 -fno-strict-aliasing -g -mriscv=RV64IMAFD -nostdinc -isystem /home/jwseo/workspace/MTE/riscv/lowrisc-chip/riscv/lib/gcc/riscv-linux/4.6.1/include -isystem /home/jwseo/workspace/MTE/riscv/lowrisc-chip/riscv/lib/gcc/riscv-linux/4.6.1/include-fixed -isystem /home/jwseo/workspace/MTE/riscv/lowrisc-chip/riscv/riscv-linux/include -isystem /home/jwseo/workspace/MTE/riscv/lowrisc-chip/sysroot/usr/include
ARFLAGS=-rcs

# If LLVM_GOLD is set, also build a library for use with LTO
#
# Note that the name of the library is hardcoded in the compiler. Thus, we call
# it the same, but put it in the lto/ subdirectory. To use it, pass
# -L/path/to/softboundcets-lib/lto to the compiler, and use -flto during
# compilation and linking
ifneq ($(LLVM_GOLD),)
all: softboundcets_rt_lto softboundmpx_rt_lto
endif

softboundmpx_rt: softboundmpx.h softboundmpx-checks.c softboundmpx.c softboundmpx-wrappers.c
	clang $(MPX_FLAGS) -c softboundmpx-checks.c -o softboundmpx-checks.o
	clang $(MPX_FLAGS) -c softboundmpx.c -o softboundmpx.o
	clang $(MPX_FLAGS) -c softboundmpx-wrappers.c -o softboundmpx-wrappers.o
	ar $(ARFLAGS) libsoftboundmpx_rt.a softboundmpx.o softboundmpx-checks.o softboundmpx-wrappers.o

softboundcets_rt: softboundcets.h softboundcets-checks.c softboundcets.c softboundcets-wrappers.c 
	clang $(CFLAGS) -c softboundcets-checks.c -o softboundcets-checks.o
	clang $(CFLAGS) -c softboundcets.c -o softboundcets.o
	clang $(CFLAGS) -c softboundcets-wrappers.c -o softboundcets-wrappers.o
	ar $(ARFLAGS) libsoftboundcets_rt.a softboundcets.o softboundcets-checks.o softboundcets-wrappers.o

softboundcets_rt_lto: softboundcets.h softboundcets-checks.c softboundcets.c softboundcets-wrappers.c 
	mkdir -p lto
	mkdir -p lto_main
	clang $(CFLAGS) -flto -c softboundcets-checks.c -o lto/softboundcets-checks.lto.o
	clang $(CFLAGS) -c softboundcets.c -o lto_main/softboundcets.o
	clang $(CFLAGS) -c softboundcets-wrappers.c -o lto_main/softboundcets-wrappers.o
	llvm-ar rcs lto_main/libsoftboundcets_rt.a lto_main/softboundcets.o lto_main/softboundcets-wrappers.o
	llvm-ranlib lto_main/libsoftboundcets_rt.a



softboundmpx_rt_lto: softboundmpx.h softboundmpx-checks.c softboundmpx.c softboundmpx-wrappers.c
	clang $(MPX_FLAGS) -flto -c softboundmpx-checks.c -o lto/softboundmpx-checks.lto.o
	clang $(MPX_FLAGS) -flto -c softboundmpx.c -o lto/softboundmpx.lto.o
	clang $(MPX_FLAGS) -flto -c softboundmpx-wrappers.c -o lto/softboundmpx-wrappers.lto.o
	ar --plugin=$(LLVM_GOLD) $(ARFLAGS) lto/libsoftboundmpx_rt.a lto/softboundmpx.lto.o lto/softboundmpx-checks.lto.o lto/softboundmpx-wrappers.lto.o


softboundcets_riscv_rt_lto: softboundcets.h softboundcets-checks.c softboundcets.c softboundcets-wrappers.c 
	mkdir -p lto
	mkdir -p lto_main
	clang $(RISCV_CFLAGS) -flto -c softboundcets-checks.c -o lto/softboundcets-checks.lto.o
	clang $(RISCV_CFLAGS) -S softboundcets.c -o lto_main/softboundcets.s
	clang -target riscv64-unknown-linux-gnu -c -o lto_main/softboundcets.o lto_main/softboundcets.s
	clang $(RISCV_CFLAGS) -S softboundcets-wrappers.c -o lto_main/softboundcets-wrappers.s
	clang -target riscv64-unknown-linux-gnu -c -o lto_main/softboundcets-wrappers.o lto_main/softboundcets-wrappers.s
	llvm-ar rcs lto_main/libsoftboundcets_rt.a lto_main/softboundcets.o lto_main/softboundcets-wrappers.o
	llvm-ranlib lto_main/libsoftboundcets_rt.a

clean:
	rm -rf *.o *.a *~ lto/ lto_main/

