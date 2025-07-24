cmd_usr/built-in.o :=   rm -f usr/built-in.o; /home/luminous418/zyc-clang/bin/aarch64-linux-gnu-ar rcSTPD usr/built-in.o usr/initramfs_data.o ; scripts/mod/modpost usr/built-in.o
