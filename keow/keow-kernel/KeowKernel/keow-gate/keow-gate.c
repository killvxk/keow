/* keow-gate.dso */
void __kernel_vsyscall()
{
	asm("int $0x04");
}
void __kernel_sigreturn()
{
	asm("mov $0x77,%eax\n\t"
		"int $0x04");
}
void __kernel_rt_sigreturn()
{
	asm("mov $0xAD,%eax\n\t"
		"int $0x04");
}
