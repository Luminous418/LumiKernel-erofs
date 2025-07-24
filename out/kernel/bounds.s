	.text
	.file	"bounds.c"
	.globl	main                            // -- Begin function main
	.p2align	4
	.type	main,@function
main:                                   // @main
// %bb.0:
	//APP
	
.ascii "->NR_PAGEFLAGS 23 __NR_PAGEFLAGS"
	//NO_APP
	//APP
	
.ascii "->MAX_NR_ZONES 3 __MAX_NR_ZONES"
	//NO_APP
	//APP
	
.ascii "->NR_CPUS_BITS 3 ilog2(CONFIG_NR_CPUS)"
	//NO_APP
	//APP
	
.ascii "->SPINLOCK_SIZE 4 sizeof(spinlock_t)"
	//NO_APP
	mov	w0, wzr
	ret
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
                                        // -- End function
	.ident	"ZyC clang version 14.0.6 (https://github.com/llvm/llvm-project f28c006a5895fc0e329fe15fead81e37457cb1d1)"
	.section	".note.GNU-stack","",@progbits
