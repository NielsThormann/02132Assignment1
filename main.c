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
unsigned char is_black;
unsigned int cell_count;
int erode_iteration;
#define box 7

void count_cells(char input_file[], char output_file[]);

struct Node {
    int x;
    int y;
    struct Node *next;
};

struct Node *head = NULL;
void insert(int x, int y) {

    struct Node *new_node = (struct Node *) malloc(sizeof(struct Node));
    new_node->x = x;
    new_node->y = y;
    new_node->next = head;
    head = new_node;
}

void print_list() {
    struct Node *ptr = head;
    while (ptr != NULL) {
        printf("(%d, %d) ", ptr->x, ptr->y);
        ptr = ptr->next;
    }
}

void erode_image_recursive(unsigned char image[BMP_WIDTH][BMP_HEIGTH]);

//Apply threshold and convert gray scale isnt in use because of the combine version
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

void convert_to_grayscale_and_apply_binary_threshold(unsigned char input_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            int binary_pixel = (input_image[x][y][0] + input_image[x][y][1] + input_image[x][y][2]) / 3;
            if (binary_pixel < 90) {
                gray_image[x][y] = 0;
            } else {
                gray_image[x][y] = 255;
            }
        }
    }
}

void erode_image(unsigned char image[BMP_WIDTH][BMP_HEIGTH]) {
    unsigned char temp_image[BMP_WIDTH][BMP_HEIGTH];
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            if (!(((image[x - 1][y] == 0 || image[x + 1][y] == 0
                || image[x][y -1] == 0 || image[x][y + 1] == 0))
                || ((image[x - 1][y] == 255 && image[x-2][y] == 0
                && image[x + 1][y] == 255 && image[x+2][y] == 0
                && image[x][y - 1] == 255 && image[x][y - 2] == 0
                && image[x][y + 1] == 255 && image[x][y + 2] == 0)))) {
                temp_image[x][y] = 255;
            } else {
                temp_image[x][y] = 0;
            }
        }
    }
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            eroded_image[x][y] = temp_image[x][y];
        }
    }
}

void draw_cross(){
    struct Node *ptr = head;
    while (ptr != NULL) {
        for(int n = -5; n <= 5; n++) {
            for(int k = -5; k <= 5; k++) {
                if(!(ptr->x+n < 0 || ptr->x+n >= BMP_WIDTH || ptr->y+k < 0 || ptr->y+k >= BMP_HEIGTH)) {
                    original_image[ptr->x + n][ptr->y + k][0] = 255;
                    original_image[ptr->x + n][ptr->y + k][1] = 0;
                    original_image[ptr->x + n][ptr->y + k][2] = 0;
                }
            }
        }
        ptr = ptr->next;
    }
}

void detect_cells(unsigned char image[BMP_WIDTH][BMP_HEIGTH]) {
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            if (image[x][y] == 255) {
                is_black = 1;
                for (int n = -box; n <= box; n++) {
                    //checking if the searching box is out of bounds
                    if(x+n < 0 ||  x+n >= BMP_WIDTH) {
                        image[x+n][y]=0;
                    }
                    if(y+n < 0 || y+n >= BMP_HEIGTH) {
                        image[x][y + n] = 0;
                    }
                    //checking if the cell is isolated by a black border
                    if (image[x - box][y + n] == 255 || image[x + box][y + n] == 255
                        || image[x + n][y + box] == 255 || image[x + n][y - box]) {
                        is_black = 0;
                        break;
                    }
                }
                if (is_black) {
                    //make everything in box black
                    for (int n = -box; n <= box; n++) {
                        for (int k = -box; k <= box; k++) {
                            if(!(x+n < 0 || x+n >= BMP_WIDTH || y+k < 0 || y+k >= BMP_HEIGTH)) {
                                image[x + n][y + k] = 0;
                            }
                        }
                    }
                    cell_count++;
                    insert(x,y);
                    //add position to list
                    //add  1 to count
                }
            }
        }
    }
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
            if (dot && (strcmp(dot, ".bmp") == 0)) {
                printf("%s\n", entry->d_name);
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
    //convert_to_grayscale(original_image);
    // Apply binary threshold to image
    //apply_binary_threshold(gray_image);
    //Convverting to grayscale and applying binary threshold in one function
    //removes 2 forloops
    convert_to_grayscale_and_apply_binary_threshold(original_image);

    // Erode image (recursively)
    //erode_image(gray_image);
    //detect_cells(eroded_image);
    //printf("Cell count = %d\n", cell_count);

    erode_iteration = 0;
    erode_image_recursive(gray_image);
    draw_cross();
    //print_list();

    // TODO : Mark cells with a cross on the image
    // TODO : Print result (how many cells and which coordinates)

    //Save image to file
    //write_bitmap_gray(eroded_image, output_file);
    write_bitmap(original_image, output_file);
}

void erode_image_recursive(unsigned char image[BMP_WIDTH][BMP_HEIGTH]) {
    erode_image(image);
    //Detect cells
    detect_cells(eroded_image);
    // Check if image is fully eroded (black)
    int is_eroded = 1;
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            if (eroded_image[x][y] == 255) {
                is_eroded = 0;
                // Using goto instead of break to break out of both loops
                goto jump;
            }
        }
    }
    jump:
    if (is_eroded) {
        printf("Image is fully eroded\n");
        printf("Cell count = %d\n", cell_count);
        return;
    }
    // Erode image again
    printf("Erode image again, %d\n", erode_iteration);
    erode_iteration++;
    erode_image_recursive(eroded_image);
}

//Main function
int main(int argc, char **argv) {
    //argc counts how may arguments are passed
    //argv[0] is a string with the name of the program
    //argv[1] is the first command line argument (input image)
    //argv[2] is the second command line argument (output image)

    printf("Beginning!\n");

    // Run all or specific test
    if (0) {
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
