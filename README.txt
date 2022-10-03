README:

There are different configurations possible when running this program.

Firstly, if not running the program from the terminal, make sure you have set the working directory to the root of the project.

The program can be ran in the default manner, where you provide an input and output file.
The only requirement here is that the “RUN_ALL” is set to 0.

The second way is to instead of specifying the input and output as a file,
you specify an input and output folder (You might have to create the output folder yourself but not the contents of course).
The requirement here is that the “RUN_ALL” is set to 1.
Running all the samples you will also have to set “WINDOWS” to either 1 or 0 depending on running on Windows or a Unix system.
This is due to the difference in the file systems and paths.
Optionally you will be able to set “BENCHMARK” to 1 or 0, which will include the timing of the total running time.
This is only available when “RUN_ALL” is set enabled.

For both configurations, it is possible to set “PRINT_POSITIONS” to either 1 or 0,
which will print the coordinates of the found cells if set to 1.

Note: We ran into some issues where sometimes the program will exit with “Invalid file depth” when running all samples,
but have all been able to run it successfully if just trying continuously some number of times.
It makes no sense why it sometimes would not work and sometimes do work.
It also seems that some of us gets this error, while some of us have not received this error.







OLD README:


This folder contains code and material for the Assignment 1.

To help you get started, we provide you with the library 'cbmp.h' + 'cbmp.c' to load and save images to bmp files. The description of the functions offered by this library is provided in the assignment document (in the Algorithm Step 1 and Step 7 descriptions).

In addition, we also provide an example code (main.c) that loads an image from a file (example.bmp), inverts the value of the color channels (i.e. creates the negative of the image), and saves it to a file. As a starting point, make sure you can run this code without error. Examine the code, and use it as starting point for implementing the algorithm.

If you use the terminal, compile and run 'main.c' as follows: 

Linux/Mac:
- To compile: gcc cbmp.c main.c -o main.out -std=c99
- To run: ./main.out example.bmp example_inv.bmp

Windows:
- To compile: gcc cbmp.c main.c -o main.exe -std=c99
- To run: main.exe example.bmp example_inv.bmp

The folder 'results_example' provides you with some example images obtained by running the algorithm. 

The folder 'samples' provides you with sample images for the 3 different levels of detection difficulty: easy, medium, and hard. Here you can also find the level 'impossible'. This level of difficulty is not part of the assignment. It is here just in case you really want to challenge your algorithm ;-)

