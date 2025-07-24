cmd_net/bpf/built-in.o :=   rm -f net/bpf/built-in.o; /home/luminous418/zyc-clang/bin/aarch64-linux-gnu-ar rcSTPD net/bpf/built-in.o net/bpf/test_run.o ; scripts/mod/modpost net/bpf/built-in.o
