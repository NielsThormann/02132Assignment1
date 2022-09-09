//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "cbmp.h"

//Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char original_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char output_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char gray_image[BMP_WIDTH][BMP_HEIGTH];
unsigned char eroded_image[BMP_WIDTH][BMP_HEIGTH];

void count_cells(char input_file[], char output_file[]);


void apply_binary_threshold(unsigned char image[BMP_WIDTH][BMP_HEIGTH]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            if (image[x][y] < 90) {
                image[x][y] = 0;
            } else {
                image[x][y] = 255;
            }
        }
    }
}

void convert_to_grayscale(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            gray_image[x][y] = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3;
        }
    }
}

void erode_image(unsigned char image[BMP_WIDTH][BMP_HEIGTH]) {
    for (int x = 1; x < BMP_WIDTH - 1; x++) {
        for (int y = 1; y < BMP_HEIGTH - 1; y++) {
            if (!(image[x - 1][y] == 0 || image[x + 1][y] == 0 || image[x][y - 1] == 0 || image[x][y + 1] == 0)) {
                eroded_image[x][y] = 255;
            }
        }
    }
    // TODO : Chec if image is fully eroded
    // is_eroded();
}

// Partly inspired by https://c-for-dummies.com/blog/?p=3246
void run_test(char input_directory[]) {
    DIR *folder;
    struct dirent *entry;

    folder = opendir(input_directory);
    if (folder == NULL) {
        return;
    }

    while ((entry = readdir(folder))) {
        // Skip the current (.) and parent (..) folders
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
            // https://stackoverflow.com/questions/32179494/how-to-find-the-files-with-done-extension-from-a-specific-directory-using-c-lan
            char *dot = strrchr(entry->d_name, '.');
            if (dot && (strcmp(dot, ".bmp") == 0))
            {
                printf ("%s\n", entry->d_name);
            }
            char input_path[100] = {0};

            // Add the next
            strcat(input_path, input_directory);
            strcat(input_path, "/");
            strcat(input_path, entry->d_name);
            // Recursively get files
            run_test(input_path);
        }
    }
    closedir(folder);
}

// Run the count cells algorithm
void count_cells(char input_file[], char output_file[]) {
    //Load image from file
    read_bitmap(input_file, original_image);

    // Convert image to grayscale
    convert_to_grayscale(original_image);

    // Apply binary threshold to image
    apply_binary_threshold(gray_image);

    // Erode image (recursively)
    erode_image(gray_image);


    // TODO : Mark cells with a cross on the image
    // TODO : Print result (how many cells and which coordinates)

    //Save image to file
    write_bitmap_gray(eroded_image, output_file);
    //write_bitmap(output_image, argv[2]);

}

//Main function
int main(int argc, char **argv) {
    //argc counts how may arguments are passed
    //argv[0] is a string with the name of the program
    //argv[1] is the first command line argument (input image)
    //argv[2] is the second command line argument (output image)

    printf("Beginning!\n");

    // Run all or specific test
    if (1) {
        run_test("../samples");
    } else {
        //Checking that 2 arguments are passed
        if (argc != 3) {
            fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
            exit(1);
        }

        count_cells(argv[1], argv[2]);
    }
    printf("Done!\n");


    return 0;
}
