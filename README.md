# Linux Shell

An interactive Linux shell implementation supporting concurrent process execution and a range of standard command operators.

__Supported features:__
- Read data from a file: ```< filename```
- Write data to a file, erasing its previous contents: ```> filename```
- Write data to the end of the file: ```>> filename```
- Background process execution: ```process &```
- Conditional execution:
    - ``` command_1 && command_2 ```: execute the second command on success of the first one
    - ``` command_1 ││ command_2 ```: execute the second command on failure of the first one
- Pipeline: ``` process_1 │ process_2 │ ... │ process_n ``` <br> The output of each process serves as input for the next process. The output of the final process is standard.

__Operator precedence:__
1. ```│```, ```>```, ```>>```, ```<```
2. ```&&```, ```││```
3. ```;```, ```&```

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

# To run the interpreter on Ubuntu:
1. Launch the terminal from the folder containing the _Shell.c_ file.
3. Input the following lines:
```
gcc Shell.c -o Shell
```
```
./Shell
```
