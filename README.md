# emugaboy

Q: What's done at the moment?  
A: All the CPU's instructions are implemented, and they pass Blargg's tests. The emulator supports ROMs without a MBC ship. Timers, audio, and joypad are not supported.

## Building
The project requires g++ with support of C++17.
The project requires SDL2 to be installed in your system.

**Enter the following commands to build for Linux**
```
cd root_project_directory
mkdir bin
make
```

Now, you are ready for testing the program.  
ROMs are placed in the 'res/roms' folder. A path to certain ROM is set in main.cpp  
Type 'make run' to run the program.

## Screenshots:
#### Tests
![alt tag](/res/images/cpu_tests/01-special.png)
![alt tag](/res/images/cpu_tests/03-op_sp,hl.png)
![alt tag](/res/images/cpu_tests/04-op_r,imm.png)
![alt tag](/res/images/cpu_tests/05-op_rp.png)
![alt tag](/res/images/cpu_tests/06-ld_r,r.png)
![alt tag](/res/images/cpu_tests/07-jr,jp,call,ret,rst.png)
![alt tag](/res/images/cpu_tests/08-misc_instrs.png)
![alt tag](/res/images/cpu_tests/09-op_r,r.png)
![alt tag](/res/images/cpu_tests/10-bit_ops.png)
![alt tag](/res/images/cpu_tests/11-op_a,hl.png)
#### Games
![alt tag](/res/images/tetris/example_1.png)
![alt tag](/res/images/tetris/example_2.png)
![alt tag](/res/images/tetris/example_3.png)

![alt tag](/res/images/dr_mario/example_1.png)
![alt tag](/res/images/dr_mario/example_2.png)

![alt tag](/res/images/kwirk/example_1.png)
![alt tag](/res/images/kwirk/example_2.png)

