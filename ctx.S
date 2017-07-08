#if  defined(__amd64__) || defined(__x86_64__)

##
#  @brief save_context
##
	.text
	.align 4
	.globl save_context
	.type save_context, @function
save_context:
	pop  %rsi			
	xorl %eax,%eax	
	movq %rbx,(%rdi)
	movq %rsp,8(%rdi)
	push %rsi	
	movq %rbp,16(%rdi)
	movq %r12,24(%rdi)
	movq %r13,32(%rdi)
	movq %r14,40(%rdi)
	movq %r15,48(%rdi)
	movq %rsi,56(%rdi)	
	ret

	.size save_context,.-save_context

##
#  @brief restore_context
##
	.text
	.align 4
	.globl restore_context
	.type restore_context, @function
restore_context:
	movl $1,%eax			
	movq (%rdi),%rbx
	movq 8(%rdi),%rsp
	movq 16(%rdi),%rbp
	movq 24(%rdi),%r12
	movq 32(%rdi),%r13
	movq 40(%rdi),%r14
	movq 48(%rdi),%r15
	jmp *56(%rdi)

	.size restore_context,.-restore_context

#else
#error "Linux cpu arch not supported"
#endif
