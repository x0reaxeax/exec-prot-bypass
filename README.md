# Executable Space Protection Bypass

This POC demonstrates execution of bytes located in supposedly non-executable region of binary, therefore completely bypassing executable-space protection.  

The root cause of this can be found here:
https://github.com/torvalds/linux/blob/master/arch/x86/include/asm/elf.h#L280  

## Brief
As it turns out, binary files built on either systems lacking NX or IA32 systems with NX, which do NOT contain the `PT_GNU_STACK` header will be marked with `exec-all`.  
This allows complete for RWX to/from everywhere in the binary.  


To achieve this, we use "historical" building tools.  
In this case, gcc 3.2.2 running on x86 Slackware9 with Linux 2.4.20  
We will end up with a binary file which can be executed on modern Linux systems, in this case **Linux 5.16.1**

The very same effect MIGHT be achievable with specific linker arguments/scripts, although I have NOT verified this.

The following code will copy assembled bytes of function `dummy()` to character array `harmless_str_buf` and execute the destination array as function.  

[Demo with reverse shell](https://youtu.be/zj5z7eB_frk)

# *** DISCLAIMER ***
This demonstration serves completely for educational purposes.
Under no circumstances can the author of this code be held responsible
for any direct or indirect damage caused by misusing any provided code and/or information. 

See [LICENSE](https://github.com/x0reaxeax/exec-prot-bypass/blob/main/LICENSE) for more details
