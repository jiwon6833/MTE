all: softboundcets_rt_lto

CFLAGS=-Wall -pedantic  -D__SOFTBOUNDCETS_SPATIAL -O2 -fno-strict-aliasing -g # gykim spatial
RISCV_CFLAGS=-target riscv64-unknown-linux-gnu -Wall -pedantic  -D__SOFTBOUNDCETS_SPATIAL -O2 -fno-strict-aliasing -mriscv=RV64IMAFD -nostdinc -isystem /home/jwseo/workspace/MTE/riscv/lowrisc-chip/riscv/lib/gcc/riscv-linux/4.6.1/include -isystem /home/jwseo/workspace/MTE/riscv/lowrisc-chip/riscv/lib/gcc/riscv-linux/4.6.1/include-fixed -isystem /home/jwseo/workspace/MTE/riscv/lowrisc-chip/sysroot/usr/include -isystem /home/jwseo/workspace/MTE/riscv/lowrisc-chip/riscv/riscv-linux/include  -DRISCV

softboundcets_rt_lto: softboundcets.h softboundcets-checks.c softboundcets.c softboundcets-wrappers.c 
	mkdir -p lto
	mkdir -p lto_main
	clang $(CFLAGS) -flto -c softboundcets-checks.c -o lto/softboundcets-checks.lto.o
	clang $(CFLAGS) -c softboundcets.c -o lto_main/softboundcets.o
	clang $(CFLAGS) -c softboundcets-wrappers.c -o lto_main/softboundcets-wrappers.o
	llvm-ar rcs lto_main/libsoftboundcets_rt.a lto_main/softboundcets.o lto_main/softboundcets-wrappers.o
	llvm-ranlib lto_main/libsoftboundcets_rt.a

softboundcets_rt_lto_dbg: softboundcets.h softboundcets-checks.c softboundcets.c softboundcets-wrappers.c stat.cc
	mkdir -p lto
	mkdir -p lto_main
	clang $(CFLAGS) -DMTE_DEBUG -flto -c softboundcets-checks.c -o lto/softboundcets-checks.lto.o
	clang $(CFLAGS) -DMTE_DEBUG -c softboundcets.c -o lto_main/softboundcets.o
	clang $(CFLAGS) -DMTE_DEBUG -c softboundcets-wrappers.c -o lto_main/softboundcets-wrappers.o
	clang++ $(CFLAGS) -DMTE_DEBUG -c stat.cc -o lto_main/stat.o
	llvm-ar rcs lto_main/libsoftboundcets_rt.a lto_main/softboundcets.o lto_main/softboundcets-wrappers.o lto_main/stat.o
	llvm-ranlib lto_main/libsoftboundcets_rt.a

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

