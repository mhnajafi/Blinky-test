/* auto-generated by gen_syscalls.py, don't edit */

#ifndef Z_INCLUDE_SYSCALLS_RESET_H
#define Z_INCLUDE_SYSCALLS_RESET_H


#include <zephyr/tracing/tracing_syscall.h>

#ifndef _ASMLANGUAGE

#include <stdarg.h>

#include <zephyr/syscall_list.h>
#include <zephyr/syscall.h>

#include <zephyr/linker/sections.h>


#ifdef __cplusplus
extern "C" {
#endif

extern int z_impl_reset_status(const struct device * dev, uint32_t id, uint8_t * status);

__pinned_func
static inline int reset_status(const struct device * dev, uint32_t id, uint8_t * status)
{
#ifdef CONFIG_USERSPACE
	if (z_syscall_trap()) {
		union { uintptr_t x; const struct device * val; } parm0 = { .val = dev };
		union { uintptr_t x; uint32_t val; } parm1 = { .val = id };
		union { uintptr_t x; uint8_t * val; } parm2 = { .val = status };
		return (int) arch_syscall_invoke3(parm0.x, parm1.x, parm2.x, K_SYSCALL_RESET_STATUS);
	}
#endif
	compiler_barrier();
	return z_impl_reset_status(dev, id, status);
}

#if defined(CONFIG_TRACING_SYSCALL)
#ifndef DISABLE_SYSCALL_TRACING

#define reset_status(dev, id, status) ({ 	int syscall__retval; 	sys_port_trace_syscall_enter(K_SYSCALL_RESET_STATUS, reset_status, dev, id, status); 	syscall__retval = reset_status(dev, id, status); 	sys_port_trace_syscall_exit(K_SYSCALL_RESET_STATUS, reset_status, dev, id, status, syscall__retval); 	syscall__retval; })
#endif
#endif


extern int z_impl_reset_line_assert(const struct device * dev, uint32_t id);

__pinned_func
static inline int reset_line_assert(const struct device * dev, uint32_t id)
{
#ifdef CONFIG_USERSPACE
	if (z_syscall_trap()) {
		union { uintptr_t x; const struct device * val; } parm0 = { .val = dev };
		union { uintptr_t x; uint32_t val; } parm1 = { .val = id };
		return (int) arch_syscall_invoke2(parm0.x, parm1.x, K_SYSCALL_RESET_LINE_ASSERT);
	}
#endif
	compiler_barrier();
	return z_impl_reset_line_assert(dev, id);
}

#if defined(CONFIG_TRACING_SYSCALL)
#ifndef DISABLE_SYSCALL_TRACING

#define reset_line_assert(dev, id) ({ 	int syscall__retval; 	sys_port_trace_syscall_enter(K_SYSCALL_RESET_LINE_ASSERT, reset_line_assert, dev, id); 	syscall__retval = reset_line_assert(dev, id); 	sys_port_trace_syscall_exit(K_SYSCALL_RESET_LINE_ASSERT, reset_line_assert, dev, id, syscall__retval); 	syscall__retval; })
#endif
#endif


extern int z_impl_reset_line_deassert(const struct device * dev, uint32_t id);

__pinned_func
static inline int reset_line_deassert(const struct device * dev, uint32_t id)
{
#ifdef CONFIG_USERSPACE
	if (z_syscall_trap()) {
		union { uintptr_t x; const struct device * val; } parm0 = { .val = dev };
		union { uintptr_t x; uint32_t val; } parm1 = { .val = id };
		return (int) arch_syscall_invoke2(parm0.x, parm1.x, K_SYSCALL_RESET_LINE_DEASSERT);
	}
#endif
	compiler_barrier();
	return z_impl_reset_line_deassert(dev, id);
}

#if defined(CONFIG_TRACING_SYSCALL)
#ifndef DISABLE_SYSCALL_TRACING

#define reset_line_deassert(dev, id) ({ 	int syscall__retval; 	sys_port_trace_syscall_enter(K_SYSCALL_RESET_LINE_DEASSERT, reset_line_deassert, dev, id); 	syscall__retval = reset_line_deassert(dev, id); 	sys_port_trace_syscall_exit(K_SYSCALL_RESET_LINE_DEASSERT, reset_line_deassert, dev, id, syscall__retval); 	syscall__retval; })
#endif
#endif


extern int z_impl_reset_line_toggle(const struct device * dev, uint32_t id);

__pinned_func
static inline int reset_line_toggle(const struct device * dev, uint32_t id)
{
#ifdef CONFIG_USERSPACE
	if (z_syscall_trap()) {
		union { uintptr_t x; const struct device * val; } parm0 = { .val = dev };
		union { uintptr_t x; uint32_t val; } parm1 = { .val = id };
		return (int) arch_syscall_invoke2(parm0.x, parm1.x, K_SYSCALL_RESET_LINE_TOGGLE);
	}
#endif
	compiler_barrier();
	return z_impl_reset_line_toggle(dev, id);
}

#if defined(CONFIG_TRACING_SYSCALL)
#ifndef DISABLE_SYSCALL_TRACING

#define reset_line_toggle(dev, id) ({ 	int syscall__retval; 	sys_port_trace_syscall_enter(K_SYSCALL_RESET_LINE_TOGGLE, reset_line_toggle, dev, id); 	syscall__retval = reset_line_toggle(dev, id); 	sys_port_trace_syscall_exit(K_SYSCALL_RESET_LINE_TOGGLE, reset_line_toggle, dev, id, syscall__retval); 	syscall__retval; })
#endif
#endif


#ifdef __cplusplus
}
#endif

#endif
#endif /* include guard */