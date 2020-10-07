.type GetSystemContext, @function
.global GetSystemContext
GetSystemContextAsm:
	# Save the return address and stack pointer.
	movq (%rsp), %r8
	movq %r8, 8*0(%rdi) // RIP
	leaq 8(%rsp), %r8
	movq %r8, 8*1(%rdi) // RSP
	# Save preserved registers.
	movq %rbx, 8*2(%rdi)
	movq %rbp, 8*3(%rdi)
	movq %r12, 8*4(%rdi)
	movq %r13, 8*5(%rdi)
	movq %r14, 8*6(%rdi)
	movq %r15, 8*7(%rdi)
	# Return.
	xorl %eax, %eax
	ret

.type SetSystemContext, @function
.global SetSystemContext
SetSystemContextAsm:
	# Should return to the address in the context.
	movq 8*0(%rdi), %r8
	# Load new stack pointer.
	movq 8*1(%rdi), %rsp
	# Load preserved registers.
	movq 8*2(%rdi), %rbx
	movq 8*3(%rdi), %rbp
	movq 8*4(%rdi), %r12
	movq 8*5(%rdi), %r13
	movq 8*6(%rdi), %r14
	movq 8*7(%rdi), %r15
	# Push RIP to stack for RET.
	pushq %r8
	# Return.
	xorl %eax, %eax
	ret

.type SwapSystemContext, @function
.global SwapSystemContext
SwapSystemContext:
	# Save the return address.
	movq (%rsp), %r8
	movq %r8, 8*0(%rdi) // RIP
	leaq 8(%rsp), %r8
	movq %r8, 8*1(%rdi) // RSP
	# Save preserved registers.
	movq %rbx, 8*2(%rdi)
	movq %rbp, 8*3(%rdi)
	movq %r12, 8*4(%rdi)
	movq %r13, 8*5(%rdi)
	movq %r14, 8*6(%rdi)
	movq %r15, 8*7(%rdi)
	# Should return to the address set in the 'to' context.
	movq 8*0(%rsi), %r8
	# Load new stack pointer.
	movq 8*1(%rsi), %rsp
	# Load preserved registers.
	movq 8*2(%rsi), %rbx
	movq 8*3(%rsi), %rbp
	movq 8*4(%rsi), %r12
	movq 8*5(%rsi), %r13
	movq 8*6(%rsi), %r14
	movq 8*7(%rsi), %r15
	# Push RIP to stack for RET.
	pushq %r8
	# Return.
	xorl %eax, %eax
	ret

.type DoRunSystemContextAsm, @function
.global DoRunSystemContext
DoRunSystemContext:
	# Save the return address.
	movq (%rsp), %r8
	movq %r8, 8*0(%rdi) // RIP
	leaq 8(%rsp), %r8
	movq %r8, 8*1(%rdi) // RSP
	# Save preserved registers.
	movq %rbx, 8*2(%rdi)
	movq %rbp, 8*3(%rdi)
	movq %r12, 8*4(%rdi)
	movq %r13, 8*5(%rdi)
	movq %r14, 8*6(%rdi)
	movq %r15, 8*7(%rdi)
	# Should return to the address set in the 'to' context.
	movq 8*0(%rsi), %r8
	# Load new stack pointer.
	movq 8*1(%rsi), %rsp
	# Set the argument.
	movq %rdx, %rdi
	# Push RIP to stack for RET.
	pushq %r8
	# Return.
	xorl %eax, %eax
	ret
