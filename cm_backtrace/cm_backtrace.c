/*
 * This file is part of the CmBacktrace Library.
 *
 * Copyright (c) 2016-2017, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Initialize function and other general function.
 * Created on: 2016-12-15
 */

#include "stdbool.h"
#include "string.h"
#include "stdio.h"

#include "cmb_cfg.h"
#include "cmb_def.h"
#include "cm_backtrace.h"


#if __STDC_VERSION__ < 199901L
    #error "must be C99 or higher. try to add '-std=c99' to compile parameters"
#endif

#if defined(__CC_ARM)
    #define SECTION_START(_name_)                _name_##$$Base
    #define SECTION_END(_name_)                  _name_##$$Limit
    #define IMAGE_SECTION_START(_name_)          Image$$##_name_##$$Base
    #define IMAGE_SECTION_END(_name_)            Image$$##_name_##$$Limit
    #define CSTACK_BLOCK_START(_name_)           SECTION_START(_name_)
    #define CSTACK_BLOCK_END(_name_)             SECTION_END(_name_)
    #define CODE_SECTION_START(_name_)           IMAGE_SECTION_START(_name_)
    #define CODE_SECTION_END(_name_)             IMAGE_SECTION_END(_name_)

    extern const int CSTACK_BLOCK_START(CMB_CSTACK_BLOCK_NAME);
    extern const int CSTACK_BLOCK_END(CMB_CSTACK_BLOCK_NAME);
    extern const int CODE_SECTION_START(CMB_CODE_SECTION_NAME);
    extern const int CODE_SECTION_END(CMB_CODE_SECTION_NAME);
#elif defined(__ICCARM__)
    #pragma section=CMB_CSTACK_BLOCK_NAME
    #pragma section=CMB_CODE_SECTION_NAME
#elif defined(__GNUC__)
    extern const int CMB_CSTACK_BLOCK_START;
    extern const int CMB_CSTACK_BLOCK_END;
    extern const int CMB_CODE_SECTION_START;
    extern const int CMB_CODE_SECTION_END;
#else
    #error "not supported compiler"
#endif

enum {
    PRINT_ASSERT_ON_THREAD,
    PRINT_ASSERT_ON_HANDLER,
    PRINT_THREAD_STACK_INFO,
    PRINT_MAIN_STACK_INFO,
    PRINT_THREAD_STACK_OVERFLOW,
    PRINT_MAIN_STACK_OVERFLOW,
    PRINT_CALL_STACK_INFO,
    PRINT_CALL_STACK_ERR,
    PRINT_FAULT_ON_THREAD,
    PRINT_FAULT_ON_HANDLER,
    PRINT_REGS_TITLE,
    PRINT_HFSR_VECTBL,
    PRINT_MFSR_IACCVIOL,
    PRINT_MFSR_DACCVIOL,
    PRINT_MFSR_MUNSTKERR,
    PRINT_MFSR_MSTKERR,
    PRINT_MFSR_MLSPERR,
    PRINT_BFSR_IBUSERR,
    PRINT_BFSR_PRECISERR,
    PRINT_BFSR_IMPREISERR,
    PRINT_BFSR_UNSTKERR,
    PRINT_BFSR_STKERR,
    PRINT_BFSR_LSPERR,
    PRINT_UFSR_UNDEFINSTR,
    PRINT_UFSR_INVSTATE,
    PRINT_UFSR_INVPC,
    PRINT_UFSR_NOCP,
    PRINT_UFSR_UNALIGNED,
    PRINT_UFSR_DIVBYZERO0,
    PRINT_DFSR_HALTED,
    PRINT_DFSR_BKPT,
    PRINT_DFSR_DWTTRAP,
    PRINT_DFSR_VCATCH,
    PRINT_DFSR_EXTERNAL,
    PRINT_MMAR,
    PRINT_BFAR,
};

static const char * const print_info[] = {
#if (CMB_PRINT_LANGUAGE == CMB_PRINT_LANGUAGE_ENGLISH)
        [PRINT_ASSERT_ON_THREAD]      = CMB_NEW_LINE"*** Assert on thread \"%s\": %s, %s, %u"CMB_NEW_LINE,
        [PRINT_ASSERT_ON_HANDLER]     = CMB_NEW_LINE"*** Assert on interrupt or bare-metal(no OS): %s, %s, %u"CMB_NEW_LINE,
        [PRINT_THREAD_STACK_INFO]     = "===== Thread stack information ====="CMB_NEW_LINE,
        [PRINT_MAIN_STACK_INFO]       = "====== Main stack information ======"CMB_NEW_LINE,
        [PRINT_THREAD_STACK_OVERFLOW] = "Error: Thread stack(%08x) was overflow"CMB_NEW_LINE,
        [PRINT_MAIN_STACK_OVERFLOW]   = "Error: Main stack(%08x) was overflow"CMB_NEW_LINE,
        [PRINT_CALL_STACK_INFO]       = "Show more call stack info by run: addr2line -e NAME%s -s -f %.*s"CMB_NEW_LINE,
        [PRINT_CALL_STACK_ERR]        = "Dump call stack has an error"CMB_NEW_LINE,
        [PRINT_FAULT_ON_THREAD]       = CMB_NEW_LINE"*** Fault on thread \"%s\""CMB_NEW_LINE,
        [PRINT_FAULT_ON_HANDLER]      = CMB_NEW_LINE"*** Fault on interrupt or bare metal(no OS)"CMB_NEW_LINE,
        [PRINT_REGS_TITLE]            = "=================== Registers information ===================="CMB_NEW_LINE,
        [PRINT_HFSR_VECTBL]           = "Hard fault is caused by failed vector fetch"CMB_NEW_LINE,
        [PRINT_MFSR_IACCVIOL]         = "Memory management fault is caused by instruction access violation"CMB_NEW_LINE,
        [PRINT_MFSR_DACCVIOL]         = "Memory management fault is caused by data access violation"CMB_NEW_LINE,
        [PRINT_MFSR_MUNSTKERR]        = "Memory management fault is caused by unstacking error"CMB_NEW_LINE,
        [PRINT_MFSR_MSTKERR]          = "Memory management fault is caused by stacking error"CMB_NEW_LINE,
        [PRINT_MFSR_MLSPERR]          = "Memory management fault is caused by floating-point lazy state preservation"CMB_NEW_LINE,
        [PRINT_BFSR_IBUSERR]          = "Bus fault is caused by instruction access violation"CMB_NEW_LINE,
        [PRINT_BFSR_PRECISERR]        = "Bus fault is caused by precise data access violation"CMB_NEW_LINE,
        [PRINT_BFSR_IMPREISERR]       = "Bus fault is caused by imprecise data access violation"CMB_NEW_LINE,
        [PRINT_BFSR_UNSTKERR]         = "Bus fault is caused by unstacking error"CMB_NEW_LINE,
        [PRINT_BFSR_STKERR]           = "Bus fault is caused by stacking error"CMB_NEW_LINE,
        [PRINT_BFSR_LSPERR]           = "Bus fault is caused by floating-point lazy state preservation"CMB_NEW_LINE,
        [PRINT_UFSR_UNDEFINSTR]       = "Usage fault is caused by attempts to execute an undefined instruction"CMB_NEW_LINE,
        [PRINT_UFSR_INVSTATE]         = "Usage fault is caused by attempts to switch to an invalid state (e.g., ARM)"CMB_NEW_LINE,
        [PRINT_UFSR_INVPC]            = "Usage fault is caused by attempts to do an exception with a bad value in the EXC_RETURN number"CMB_NEW_LINE,
        [PRINT_UFSR_NOCP]             = "Usage fault is caused by attempts to execute a coprocessor instruction"CMB_NEW_LINE,
        [PRINT_UFSR_UNALIGNED]        = "Usage fault is caused by indicates that an unaligned access fault has taken place"CMB_NEW_LINE,
        [PRINT_UFSR_DIVBYZERO0]       = "Usage fault is caused by Indicates a divide by zero has taken place (can be set only if DIV_0_TRP is set)"CMB_NEW_LINE,
        [PRINT_DFSR_HALTED]           = "Debug fault is caused by halt requested in NVIC"CMB_NEW_LINE,
        [PRINT_DFSR_BKPT]             = "Debug fault is caused by BKPT instruction executed"CMB_NEW_LINE,
        [PRINT_DFSR_DWTTRAP]          = "Debug fault is caused by DWT match occurred"CMB_NEW_LINE,
        [PRINT_DFSR_VCATCH]           = "Debug fault is caused by Vector fetch occurred"CMB_NEW_LINE,
        [PRINT_DFSR_EXTERNAL]         = "Debug fault is caused by EDBGRQ signal asserted"CMB_NEW_LINE,
        [PRINT_MMAR]                  = "The memory management fault occurred address is %08x"CMB_NEW_LINE,
        [PRINT_BFAR]                  = "The bus fault occurred address is %08x"CMB_NEW_LINE,
#elif (CMB_PRINT_LANGUAGE == CMB_PRINT_LANGUAGE_CHINESE)
        [PRINT_ASSERT_ON_THREAD]      = CMB_NEW_LINE"*** 在线程 \"%s\" 中发生断言: %s, %s, %u"CMB_NEW_LINE,
        [PRINT_ASSERT_ON_HANDLER]     = CMB_NEW_LINE"*** 在中断或裸机环境下发生断言: %s, %s, %u"CMB_NEW_LINE,
        [PRINT_THREAD_STACK_INFO]     = "=========== 线程堆栈信息 ==========="CMB_NEW_LINE,
        [PRINT_MAIN_STACK_INFO]       = "============ 主堆栈信息 ============"CMB_NEW_LINE,
        [PRINT_THREAD_STACK_OVERFLOW] = "错误：线程栈(%08x)发生溢出"CMB_NEW_LINE,
        [PRINT_MAIN_STACK_OVERFLOW]   = "错误：主栈(%08x)发生溢出"CMB_NEW_LINE,
        [PRINT_CALL_STACK_INFO]       = "查看更多函数调用栈信息，请运行：addr2line -e NAME%s -s -f %.*s"CMB_NEW_LINE,
        [PRINT_CALL_STACK_ERR]        = "获取函数调用栈失败"CMB_NEW_LINE,
        [PRINT_FAULT_ON_THREAD]       = CMB_NEW_LINE"*** 在线程 \"%s\" 中发生错误异常"CMB_NEW_LINE,
        [PRINT_FAULT_ON_HANDLER]      = CMB_NEW_LINE"*** 在中断或裸机环境下发生错误异常"CMB_NEW_LINE,
        [PRINT_REGS_TITLE]            = "========================= 寄存器信息 ========================="CMB_NEW_LINE,
        [PRINT_HFSR_VECTBL]           = "发生硬错误，原因：取中断向量时出错"CMB_NEW_LINE,
        [PRINT_MFSR_IACCVIOL]         = "发生存储器管理错误，原因：企图从不允许访问的区域取指令"CMB_NEW_LINE,
        [PRINT_MFSR_DACCVIOL]         = "发生存储器管理错误，原因：企图从不允许访问的区域读、写数据"CMB_NEW_LINE,
        [PRINT_MFSR_MUNSTKERR]        = "发生存储器管理错误，原因：出栈时企图访问不被允许的区域"CMB_NEW_LINE,
        [PRINT_MFSR_MSTKERR]          = "发生存储器管理错误，原因：入栈时企图访问不被允许的区域"CMB_NEW_LINE,
        [PRINT_MFSR_MLSPERR]          = "发生存储器管理错误，原因：惰性保存浮点状态时发生错误"CMB_NEW_LINE,
        [PRINT_BFSR_IBUSERR]          = "发生总线错误，原因：指令总线错误"CMB_NEW_LINE,
        [PRINT_BFSR_PRECISERR]        = "发生总线错误，原因：精确的数据总线错误"CMB_NEW_LINE,
        [PRINT_BFSR_IMPREISERR]       = "发生总线错误，原因：不精确的数据总线错误"CMB_NEW_LINE,
        [PRINT_BFSR_UNSTKERR]         = "发生总线错误，原因：出栈时发生错误"CMB_NEW_LINE,
        [PRINT_BFSR_STKERR]           = "发生总线错误，原因：入栈时发生错误"CMB_NEW_LINE,
        [PRINT_BFSR_LSPERR]           = "发生总线错误，原因：惰性保存浮点状态时发生错误"CMB_NEW_LINE,
        [PRINT_UFSR_UNDEFINSTR]       = "发生用法错误，原因：企图执行未定义指令"CMB_NEW_LINE,
        [PRINT_UFSR_INVSTATE]         = "发生用法错误，原因：试图切换到 ARM 状态"CMB_NEW_LINE,
        [PRINT_UFSR_INVPC]            = "发生用法错误，原因：无效的异常返回码"CMB_NEW_LINE,
        [PRINT_UFSR_NOCP]             = "发生用法错误，原因：企图执行协处理器指令"CMB_NEW_LINE,
        [PRINT_UFSR_UNALIGNED]        = "发生用法错误，原因：企图执行非对齐访问"CMB_NEW_LINE,
        [PRINT_UFSR_DIVBYZERO0]       = "发生用法错误，原因：企图执行除 0 操作"CMB_NEW_LINE,
        [PRINT_DFSR_HALTED]           = "发生调试错误，原因：NVIC 停机请求"CMB_NEW_LINE,
        [PRINT_DFSR_BKPT]             = "发生调试错误，原因：执行 BKPT 指令"CMB_NEW_LINE,
        [PRINT_DFSR_DWTTRAP]          = "发生调试错误，原因：数据监测点匹配"CMB_NEW_LINE,
        [PRINT_DFSR_VCATCH]           = "发生调试错误，原因：发生向量捕获"CMB_NEW_LINE,
        [PRINT_DFSR_EXTERNAL]         = "发生调试错误，原因：外部调试请求"CMB_NEW_LINE,
        [PRINT_MMAR]                  = "发生存储器管理错误的地址：%08x"CMB_NEW_LINE,
        [PRINT_BFAR]                  = "发生总线错误的地址：%08x"CMB_NEW_LINE,
#else
    #error "CMB_PRINT_LANGUAGE defined error in 'cmb_cfg.h'"
#endif
};

static uint32_t  main_stack_start_addr = 0;
static size_t    main_stack_size = 0;
static uint32_t  code_start_addr = 0;
static size_t    code_size = 0;
static bool      init_ok = false;
static bool      on_assert = false;
static bool      on_fault = false;
static bool      stack_is_overflow = false;
static bool      on_thread_before_fault = false;
#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7)
static bool      statck_has_fpu_regs = false;
#endif
static uint32_t  assert_stack_size = 0;
static struct cmb_hard_fault_regs   regs;


/**
 * library initialize
 */
void cm_backtrace_init(void) {
#if defined(__CC_ARM)
    main_stack_start_addr = (uint32_t)&CSTACK_BLOCK_START(CMB_CSTACK_BLOCK_NAME);
    main_stack_size = (uint32_t)&CSTACK_BLOCK_END(CMB_CSTACK_BLOCK_NAME) - main_stack_start_addr;
    code_start_addr = (uint32_t)&CODE_SECTION_START(CMB_CODE_SECTION_NAME);
    code_size = (uint32_t)&CODE_SECTION_END(CMB_CODE_SECTION_NAME) - code_start_addr;
#elif defined(__ICCARM__)
    main_stack_start_addr = (uint32_t)__section_begin(CMB_CSTACK_BLOCK_NAME);
    main_stack_size = (uint32_t)__section_end(CMB_CSTACK_BLOCK_NAME) - main_stack_start_addr;
    code_start_addr = (uint32_t)__section_begin(CMB_CODE_SECTION_NAME);
    code_size = (uint32_t)__section_end(CMB_CODE_SECTION_NAME) - code_start_addr;
#elif defined(__GNUC__)
    main_stack_start_addr = (uint32_t)(&CMB_CSTACK_BLOCK_START);
    main_stack_size = (uint32_t)(&CMB_CSTACK_BLOCK_END) - main_stack_start_addr;
    code_start_addr = (uint32_t)(&CMB_CODE_SECTION_START);
    code_size = (uint32_t)(&CMB_CODE_SECTION_END) - code_start_addr;
#else
    #error "not supported compiler"
#endif

    assert_stack_size = cmb_get_sp();
    cm_backtrace_assert("", "", 0);

    init_ok = true;
}

#if (CMB_OS_PLATFORM_TYPE > CMB_OS_PLATFORM_NONE)
/**
 * Get current thread stack information
 *
 * @param sp stack current pointer
 * @param start_addr stack start address
 * @param size stack size
 */
static void get_cur_thread_stack_info(uint32_t sp, uint32_t *start_addr, size_t *size) {
    CMB_ASSERT(start_addr);
    CMB_ASSERT(size);

#if (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_RTT)

    *start_addr = (uint32_t) rt_thread_self()->stack_addr;
    *size = rt_thread_self()->stack_size;

#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_UCOSII)

    extern OS_TCB *OSTCBCur;
    *start_addr = (uint32_t) OSTCBCur->OSTCBStkBottom;
    *size = OSTCBCur->OSTCBStkSize * sizeof(OS_STK);

#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_UCOSIII)

    extern OS_TCB *OSTCBCurPtr;
    *start_addr = (uint32_t) OSTCBCurPtr->StkBasePtr;
    *size = OSTCBCurPtr->StkSize * sizeof(CPU_STK_SIZE);

#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_FREERTOS)

    *start_addr = (uint32_t)vTaskStackAddr();
    *size = vTaskStackSize() * sizeof(StackType_t);

#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_RTX5)

  //osRtxThread_t *thread = osRtxInfo.thread.run.curr;
    *start_addr = (uint32_t) osRtxInfo.thread.run.curr->stack_mem;
    *size = osRtxInfo.thread.run.curr->stack_size;

#endif
}

/**
 * Get current thread name
 */
static const char *get_cur_thread_name(void) {
    const char *name;

#if (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_RTT)

    name = rt_thread_self()->name;

#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_UCOSII)

    extern OS_TCB *OSTCBCur;
#  if OS_TASK_NAME_SIZE > 0 || OS_TASK_NAME_EN > 0
    name = (const char *)OSTCBCur->OSTCBTaskName;
#  else
    name = NULL;
#  endif /* OS_TASK_NAME_SIZE > 0 || OS_TASK_NAME_EN > 0 */

#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_UCOSIII)

    extern OS_TCB *OSTCBCurPtr;
    name = (const char *)OSTCBCurPtr->NamePtr;

#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_FREERTOS)

    name = vTaskName();

#elif (CMB_OS_PLATFORM_TYPE == CMB_OS_PLATFORM_RTX5)

  //osRtxThread_t *thread = osRtxInfo.thread.run.curr;
    if ((osRtxInfo.thread.run.curr == NULL) || (osRtxInfo.thread.run.curr->id != osRtxIdThread)) {
        name = NULL;  //lint !e{904} "Return statement before end of function" [MISRA Note 1]
    }
    else {
        name = osRtxInfo.thread.run.curr->name;
    }

#endif

    return( (name != NULL) ? name : "NO_NAME" );
}

#endif /* (CMB_OS_PLATFORM_TYPE > CMB_OS_PLATFORM_NONE) */

#if CMB_USING_DUMP_STACK_INFO
/**
 * dump current stack information
 */
static void dump_stack(uint32_t stack_addr, size_t stack_size, uint32_t *stack_pointer, bool on_thread) {
    if ((uint32_t) stack_pointer < stack_addr) {
        cmb_print(print_info[on_thread ? PRINT_THREAD_STACK_OVERFLOW : PRINT_MAIN_STACK_OVERFLOW], stack_pointer);
        stack_pointer = (uint32_t *)stack_addr;
    }
    else if ((uint32_t) stack_pointer > stack_addr + stack_size) {
        cmb_print(print_info[on_thread ? PRINT_THREAD_STACK_OVERFLOW : PRINT_MAIN_STACK_OVERFLOW], stack_pointer);
        stack_pointer = (uint32_t *)(stack_addr + stack_size);
    }
    cmb_print(print_info[on_thread ? PRINT_THREAD_STACK_INFO : PRINT_MAIN_STACK_INFO]);

    for (; (uint32_t) stack_pointer < stack_addr + stack_size; stack_pointer++) {
        cmb_print("  addr: %08x    data: %08x"CMB_NEW_LINE, stack_pointer, *stack_pointer);
    }
    cmb_print("===================================="CMB_NEW_LINE);
}
#endif /* CMB_USING_DUMP_STACK_INFO */

/**
 * backtrace function call stack
 *
 * @param buffer call stack buffer
 * @param size buffer size
 * @param sp stack pointer
 *
 * @return depth
 */
size_t cm_backtrace_call_stack(uint32_t *buffer, size_t size, uint32_t sp) {
    uint32_t stack_start_addr = main_stack_start_addr, pc;
    size_t depth = 0, stack_size = main_stack_size;
    bool regs_saved_lr_is_valid = false;

    CMB_ASSERT(init_ok);

    if (on_fault) {
        if (!stack_is_overflow) {
            /* first depth is PC */
            buffer[depth++] = regs.saved.pc;
            /* second depth is from LR, so need decrease a word to PC */
            pc = regs.saved.lr - sizeof(size_t);
            if ((pc >= code_start_addr) && (pc <= code_start_addr + code_size) && (depth < CMB_CALL_STACK_MAX_DEPTH)
                    && (depth < size)) {
                buffer[depth++] = pc;
                regs_saved_lr_is_valid = true;
            }
        }

#if (CMB_OS_PLATFORM_TYPE > CMB_OS_PLATFORM_NONE)
        /* program is running on thread before fault */
        if (on_thread_before_fault) {
            get_cur_thread_stack_info(sp, &stack_start_addr, &stack_size);
        }
    } else {
        /* OS environment */
        if (cmb_get_sp() == cmb_get_psp()) {
            get_cur_thread_stack_info(sp, &stack_start_addr, &stack_size);
        }
#endif /* (CMB_OS_PLATFORM_TYPE > CMB_OS_PLATFORM_NONE) */

    }

    if (stack_is_overflow) {
        if (sp < stack_start_addr) {
            sp = stack_start_addr;
        } else if (sp > stack_start_addr + stack_size) {
            sp = stack_start_addr + stack_size;
        }
    }

    /* copy called function address */
    for (; sp < stack_start_addr + stack_size; sp += sizeof(size_t)) {
        /* the *sp value may be LR, so need decrease a word to PC */
        pc = *((uint32_t *)sp) - sizeof(size_t);
        /* the Cortex-M using thumb instruction, so the pc must be an odd number */
        if (pc % 2 == 0) {
            continue;
        }
        if ((pc >= code_start_addr) && (pc <= code_start_addr + code_size) && (depth < CMB_CALL_STACK_MAX_DEPTH)
                && (depth < size)) {
            /* the second depth function may be already saved, so need ignore repeat */
            if ((depth == 2) && regs_saved_lr_is_valid && (pc == buffer[1])) {
                continue;
            }
            buffer[depth++] = pc;
        }
    }

    return depth;
}

/**
 * dump function call stack
 *
 * @param sp stack pointer
 */
static void print_call_stack(uint32_t sp) {
    static char      call_stack_info[CMB_CALL_STACK_MAX_DEPTH * (8 + 1)] = { 0 };
    static uint32_t  call_stack_buf[CMB_CALL_STACK_MAX_DEPTH] = { 0 };
    size_t           i, cur_depth = 0;

    cur_depth = cm_backtrace_call_stack(call_stack_buf, CMB_CALL_STACK_MAX_DEPTH, sp);

    for (i = 0; i < cur_depth; i++) {
        sprintf(call_stack_info + i * (8 + 1), "%08lx", (unsigned long)call_stack_buf[i]);
        call_stack_info[i * (8 + 1) + 8] = ' ';
    }

    if (cur_depth) {
        cmb_print(print_info[PRINT_CALL_STACK_INFO], CMB_ELF_FILE_EXTENSION_NAME, cur_depth * (8 + 1), call_stack_info);
    } else {
        cmb_print(print_info[PRINT_CALL_STACK_ERR]);
    }
}

/**
 * backtrace for assert
 *
 * @param expr  assert expression
 * @param file  file name of assert
 * @param line  line number of assert
 * @param sp the stack pointer when on assert occurred
 */
void cm_backtrace_assert(const char *expr, const char *file, unsigned line) {
#if CMB_USING_DUMP_STACK_INFO
    uint32_t  stack_start_addr;
    size_t    stack_size;
#endif
    uint32_t  stack_pointer = cmb_get_sp();

    if (!init_ok) {
        assert_stack_size -= stack_pointer;
        return;
    }
    if (on_assert) {
        return;
    }
    on_assert = true;
    CMB_ASSERT(!on_fault);
    CMB_ASSERT(expr);
    CMB_ASSERT(file);

    if (stack_pointer == cmb_get_psp()) {
        cmb_print(print_info[PRINT_ASSERT_ON_THREAD], get_cur_thread_name(), expr, file, line);
#if CMB_USING_DUMP_STACK_INFO
        get_cur_thread_stack_info(stack_pointer, &stack_start_addr, &stack_size);
        dump_stack(stack_start_addr, stack_size, (uint32_t *)(stack_pointer + assert_stack_size), true);
#endif
    }
    else {
        cmb_print(print_info[PRINT_ASSERT_ON_HANDLER], expr, file, line);
#if CMB_USING_DUMP_STACK_INFO
        dump_stack(main_stack_start_addr, main_stack_size, (uint32_t *)(stack_pointer + assert_stack_size), false);
#endif
    }

    print_call_stack(stack_pointer);
    cmb_abort();
}

#if (CMB_CPU_PLATFORM_TYPE != CMB_CPU_ARM_CORTEX_M0)
/**
 * fault diagnosis then print cause of fault
 */
static void fault_diagnosis(void) {
    if (regs.hfsr.bits.VECTBL) {
        cmb_print(print_info[PRINT_HFSR_VECTBL]);
    }
    if (regs.hfsr.bits.FORCED) {
        /* Memory Management Fault */
        if (regs.mfsr.value) {
            if (regs.mfsr.bits.IACCVIOL) {
                cmb_print(print_info[PRINT_MFSR_IACCVIOL]);
            }
            if (regs.mfsr.bits.DACCVIOL) {
                cmb_print(print_info[PRINT_MFSR_DACCVIOL]);
            }
            if (regs.mfsr.bits.MUNSTKERR) {
                cmb_print(print_info[PRINT_MFSR_MUNSTKERR]);
            }
            if (regs.mfsr.bits.MSTKERR) {
                cmb_print(print_info[PRINT_MFSR_MSTKERR]);
            }

#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7)
            if (regs.mfsr.bits.MLSPERR) {
                cmb_print(print_info[PRINT_MFSR_MLSPERR]);
            }
#endif

            if (regs.mfsr.bits.MMARVALID) {
                if (regs.mfsr.bits.IACCVIOL || regs.mfsr.bits.DACCVIOL) {
                    cmb_print(print_info[PRINT_MMAR], regs.mmar);
                }
            }
        }
        /* Bus Fault */
        if (regs.bfsr.value) {
            if (regs.bfsr.bits.IBUSERR) {
                cmb_print(print_info[PRINT_BFSR_IBUSERR]);
            }
            if (regs.bfsr.bits.PRECISERR) {
                cmb_print(print_info[PRINT_BFSR_PRECISERR]);
            }
            if (regs.bfsr.bits.IMPREISERR) {
                cmb_print(print_info[PRINT_BFSR_IMPREISERR]);
            }
            if (regs.bfsr.bits.UNSTKERR) {
                cmb_print(print_info[PRINT_BFSR_UNSTKERR]);
            }
            if (regs.bfsr.bits.STKERR) {
                cmb_print(print_info[PRINT_BFSR_STKERR]);
            }

#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7)
            if (regs.bfsr.bits.LSPERR) {
                cmb_print(print_info[PRINT_BFSR_LSPERR]);
            }
#endif

            if (regs.bfsr.bits.BFARVALID) {
                if (regs.bfsr.bits.PRECISERR) {
                    cmb_print(print_info[PRINT_BFAR], regs.bfar);
                }
            }

        }
        /* Usage Fault */
        if (regs.ufsr.value) {
            if (regs.ufsr.bits.UNDEFINSTR) {
                cmb_print(print_info[PRINT_UFSR_UNDEFINSTR]);
            }
            if (regs.ufsr.bits.INVSTATE) {
                cmb_print(print_info[PRINT_UFSR_INVSTATE]);
            }
            if (regs.ufsr.bits.INVPC) {
                cmb_print(print_info[PRINT_UFSR_INVPC]);
            }
            if (regs.ufsr.bits.NOCP) {
                cmb_print(print_info[PRINT_UFSR_NOCP]);
            }
            if (regs.ufsr.bits.UNALIGNED) {
                cmb_print(print_info[PRINT_UFSR_UNALIGNED]);
            }
            if (regs.ufsr.bits.DIVBYZERO0) {
                cmb_print(print_info[PRINT_UFSR_DIVBYZERO0]);
            }
        }
    }
    /* Debug Fault */
    if (regs.hfsr.bits.DEBUGEVT) {
        if (regs.dfsr.value) {
            if (regs.dfsr.bits.HALTED) {
                cmb_print(print_info[PRINT_DFSR_HALTED]);
            }
            if (regs.dfsr.bits.BKPT) {
                cmb_print(print_info[PRINT_DFSR_BKPT]);
            }
            if (regs.dfsr.bits.DWTTRAP) {
                cmb_print(print_info[PRINT_DFSR_DWTTRAP]);
            }
            if (regs.dfsr.bits.VCATCH) {
                cmb_print(print_info[PRINT_DFSR_VCATCH]);
            }
            if (regs.dfsr.bits.EXTERNAL) {
                cmb_print(print_info[PRINT_DFSR_EXTERNAL]);
            }
        }
    }
}
#endif /* (CMB_CPU_PLATFORM_TYPE != CMB_CPU_ARM_CORTEX_M0) */

#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7)
static uint32_t statck_del_fpu_regs(uint32_t fault_handler_lr, uint32_t sp) {
    statck_has_fpu_regs = (fault_handler_lr & (1UL << 4)) == 0 ? true : false;

    /* the stack has S0~S15 and FPSCR registers when statck_has_fpu_regs is true, double word align */
    return statck_has_fpu_regs == true ? sp + sizeof(size_t) * 18 : sp;
}
#endif

/**
 * backtrace for fault
 * @note only call once
 *
 * @param fault_handler_lr the LR register value on fault handler
 * @param fault_handler_sp the stack pointer on fault handler
 */
void cm_backtrace_fault(uint32_t fault_handler_lr, uint32_t fault_handler_sp) {
    uint32_t stack_pointer = fault_handler_sp, saved_regs_addr = stack_pointer;
    const char *regs_name[] = { "R0 ", "R1 ", "R2 ", "R3 ", "R12", "LR ", "PC ", "PSR" };

#if CMB_USING_DUMP_STACK_INFO
    uint32_t stack_start_addr = main_stack_start_addr;
    size_t stack_size = main_stack_size;
#endif

    if (on_assert) {
        return;
    }
    CMB_ASSERT(init_ok);
    /* only call once */
    CMB_ASSERT(!on_fault);
    on_fault = true;

#if (CMB_OS_PLATFORM_TYPE > CMB_OS_PLATFORM_NONE)
    on_thread_before_fault = fault_handler_lr & (1UL << 2);
    /* check which stack was used before (MSP or PSP) */
    if (on_thread_before_fault) {
        cmb_print(print_info[PRINT_FAULT_ON_THREAD], get_cur_thread_name());
        saved_regs_addr = stack_pointer = cmb_get_psp();

#if CMB_USING_DUMP_STACK_INFO
        get_cur_thread_stack_info(stack_pointer, &stack_start_addr, &stack_size);
#endif /* CMB_USING_DUMP_STACK_INFO */

    } else {
        cmb_print(print_info[PRINT_FAULT_ON_HANDLER]);
    }
#else
    /* bare metal(no OS) environment */
    cmb_print(print_info[PRINT_FAULT_ON_HANDLER]);
#endif /* (CMB_OS_PLATFORM_TYPE > CMB_OS_PLATFORM_NONE) */

    /* delete saved R0~R3, R12, LR,PC,xPSR registers space */
    stack_pointer += sizeof(size_t) * 8;

#if (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7)
    stack_pointer = statck_del_fpu_regs(fault_handler_lr, stack_pointer);
#endif /* (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M4) || (CMB_CPU_PLATFORM_TYPE == CMB_CPU_ARM_CORTEX_M7) */

#if CMB_USING_DUMP_STACK_INFO
    /* check stack overflow */
    if (stack_pointer < stack_start_addr || stack_pointer > stack_start_addr + stack_size) {
        stack_is_overflow = true;
    }
    /* dump stack information */
    dump_stack(stack_start_addr, stack_size, (uint32_t *)stack_pointer, on_thread_before_fault);
#endif /* CMB_USING_DUMP_STACK_INFO */

    /* the stack frame may be get failed when it is overflow  */
    if (!stack_is_overflow) {
        /* dump register */
        cmb_print(print_info[PRINT_REGS_TITLE]);

        regs.saved.r0        = ((uint32_t *)saved_regs_addr)[0];  // Register R0
        regs.saved.r1        = ((uint32_t *)saved_regs_addr)[1];  // Register R1
        regs.saved.r2        = ((uint32_t *)saved_regs_addr)[2];  // Register R2
        regs.saved.r3        = ((uint32_t *)saved_regs_addr)[3];  // Register R3
        regs.saved.r12       = ((uint32_t *)saved_regs_addr)[4];  // Register R12
        regs.saved.lr        = ((uint32_t *)saved_regs_addr)[5];  // Link register LR
        regs.saved.pc        = ((uint32_t *)saved_regs_addr)[6];  // Program counter PC
        regs.saved.psr.value = ((uint32_t *)saved_regs_addr)[7];  // Program status word PSR

        cmb_print("  %s: %08x  %s: %08x  %s: %08x  %s: %08x"CMB_NEW_LINE, regs_name[0], regs.saved.r0,
                                                                          regs_name[1], regs.saved.r1,
                                                                          regs_name[2], regs.saved.r2,
                                                                          regs_name[3], regs.saved.r3);
        cmb_print("  %s: %08x  %s: %08x  %s: %08x  %s: %08x"CMB_NEW_LINE, regs_name[4], regs.saved.r12,
                                                                          regs_name[5], regs.saved.lr,
                                                                          regs_name[6], regs.saved.pc,
                                                                          regs_name[7], regs.saved.psr.value);
        cmb_print("=============================================================="CMB_NEW_LINE);
    }

    /* the Cortex-M0 is not support fault diagnosis */
#if (CMB_CPU_PLATFORM_TYPE != CMB_CPU_ARM_CORTEX_M0)
    regs.syshndctrl.value = CMB_SYSHND_CTRL;  // System Handler Control and State Register
    regs.mfsr.value       = CMB_NVIC_MFSR;    // Memory Fault Status Register
    regs.mmar             = CMB_NVIC_MMAR;    // Memory Management Fault Address Register
    regs.bfsr.value       = CMB_NVIC_BFSR;    // Bus Fault Status Register
    regs.bfar             = CMB_NVIC_BFAR;    // Bus Fault Manage Address Register
    regs.ufsr.value       = CMB_NVIC_UFSR;    // Usage Fault Status Register
    regs.hfsr.value       = CMB_NVIC_HFSR;    // Hard Fault Status Register
    regs.dfsr.value       = CMB_NVIC_DFSR;    // Debug Fault Status Register
    regs.afsr             = CMB_NVIC_AFSR;    // Auxiliary Fault Status Register

    fault_diagnosis();
#endif

    print_call_stack(stack_pointer);
    cmb_abort();
}

