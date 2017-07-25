# emugaboy

Q: What's done at the moment?  
A: Almost all the CPU's instructions are implemented, and they pass Blargg's tests. The emulator supports ROMs with/without a MBC1 chip. Audio is not supported.

## Building
The project requires g++ that supports C++17.  
The project requires SDL2 to be installed in your system.

**Enter the following commands to build for Linux**
```
cd root_project_directory
mkdir bin
make
```

Now, you are ready for testing the program.  
ROMs are placed in the 'res/roms' folder. A path to a particular ROM is set in main.cpp  
Type 'make run' to run the program.

## Screenshots:
#### Tests
![alt tag](/res/images/cpu_tests/cpu_instrs.png)
![alt tag](/res/images/cpu_tests/instr_timing.png)
#### Games
![alt tag](/res/images/tetris/example_1.png)
![alt tag](/res/images/tetris/example_2.png)
![alt tag](/res/images/tetris/example_3.png)
![alt tag](/res/images/mario/example_1.png)
![alt tag](/res/images/mario/example_2.png)

![alt tag](/res/images/megaman.png)
![alt tag](/res/images/castlevania.png)
![alt tag](/res/images/kwirk/example_1.png)
![alt tag](/res/images/kwirk/example_2.png)
![alt tag](/res/images/zelda.png)

![alt tag](/res/images/battlecity/example_1.png)
![alt tag](/res/images/battlecity/example_2.png)
![alt tag](/res/images/karateka/example_1.png)
![alt tag](/res/images/karateka/example_2.png)
![alt tag](/res/images/marble_madness.png)
