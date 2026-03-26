# Linux-Shell

An interactive command prompt interpreter developed for Linux systems.

Supports concurrent process execution.

This interpreter supports the following operators:

1. '<'  - read from file
2. '>'  - write to file, erasing its previous contents
3. '>>' - write to the end of file
4. '&' - execute the command as a background process
5. '&&' (e.g.: _command\_1 && command\_2_) - execute the second command only after the successful execution of the first command
6. '││' (e.g.: _command\_1 ││ command\_2_) - execute the second command only in case of an error in the execution of the first command
7. '│' (e.g.: _process\_1 │ process\_2 │ ... │ process\_n_) - process conveyor: the output of each process serves as input for the next process. The output of the final process is standard.

The operator priority is as follows:
1. _│_, _>_, _>>_, _<_
2. _&&_, _││_
3. _;_, _&_

The parsing algorithm of the interpreter is as follows:
![CommandPrompt](https://github.com/user-attachments/assets/20f29762-fce3-4cd3-9e0a-80dece2f5739)

![ConditionalCommand](https://github.com/user-attachments/assets/06ae652d-255a-4428-8d44-5ec554ea55f8)

![Command](https://github.com/user-attachments/assets/760bcb19-d893-4b1a-ae0b-bb960d593725)

![InputOrOutputRedirection](https://github.com/user-attachments/assets/46014134-3fc4-43f9-9d4f-bbe65375416c)

![InputRedirection](https://github.com/user-attachments/assets/bbdd8c1e-1c82-4e94-9f17-f5c38c3b56ec)

![OutputRedirection](https://github.com/user-attachments/assets/c604aae0-cee8-455a-bafc-86b08befdd51)

![Conveyor](https://github.com/user-attachments/assets/bfdb6bf0-63a9-4e7a-98fe-99574801d587)

![SimpleCommand](https://github.com/user-attachments/assets/a121e756-6bf2-47a5-a110-acfd7b547d26)
\
\
_Tested on Ubuntu 20.04.1 LTS._

To run the interpreter:
1. Launch the terminal from the folder containing the _Shell.c_ file.
3. Input the following lines:
```
gcc Shell.c -o Shell
```
```
./Shell
```
