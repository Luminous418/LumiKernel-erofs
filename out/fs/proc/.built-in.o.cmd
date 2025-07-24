cmd_fs/proc/built-in.o :=   rm -f fs/proc/built-in.o; /home/luminous418/zyc-clang/bin/aarch64-linux-gnu-ar rcSTPD fs/proc/built-in.o fs/proc/proc.o ; scripts/mod/modpost fs/proc/built-in.o
