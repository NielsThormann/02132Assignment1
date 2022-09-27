//To compile (linux/mac): gcc cbmp.c main.c -o main.out -std=c99
//To run (linux/mac): ./main.out example.bmp example_inv.bmp

//To compile (win): gcc cbmp.c main.c -o main.exe -std=c99
//To run (win): main.exe example.bmp example_inv.bmp

#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include "cbmp.h"

//Declaring the array to store the image (unsigned char = unsigned 8 bit)
unsigned char original_image[BMP_WIDTH][BMP_HEIGTH][BMP_CHANNELS];
unsigned char gray_image[BMP_WIDTH][BMP_HEIGTH];
unsigned char eroded_image[BMP_WIDTH][BMP_HEIGTH];
unsigned char is_black;
unsigned int cell_count;
unsigned int total_cell_count;

#define box 7
#define PRINT_POSITIONS 0
#define RUN_ALL 1
#define BENCHMARK 1


// Linked list for position of cells
struct PositionNode {
    int x;
    int y;
    struct PositionNode *next;
};

struct PositionNode *headCross = NULL;

void insert(int x, int y) {
    struct PositionNode *new_node = (struct PositionNode *) malloc(sizeof(struct PositionNode));
    new_node->x = x;
    new_node->y = y;
    new_node->next = headCross;
    headCross = new_node;
}

// Reset the linked list
void reset_position_list() {
    struct PositionNode *ptr = headCross;
    struct PositionNode *temp;
    while (ptr != NULL) {
        temp = ptr;
        ptr = ptr->next;
        free(temp);
    }
    headCross = NULL;
}


// Linked list for input and output paths
struct pathNode {
    char input_path[100];
    char output_path[100];
    struct pathNode *next;
};

struct pathNode *head = NULL;

// Insert into list
void insert_paths(char input_path[], char output_path[]) {
    // Create a link
    struct pathNode *link = (struct pathNode *) malloc(sizeof(struct pathNode));

    strcpy(link->input_path, input_path);
    strcpy(link->output_path, output_path);

    // Point it to old first pathNode
    link->next = head;

    // Point first to new first pathNode
    head = link;
}


// Prototypes
void count_cells(char input_file[], char output_file[]);

void erode_image_recursive(unsigned char image[BMP_WIDTH][BMP_HEIGTH]);

void run_all_test_cases();

void print_cell_positions();

//Apply threshold and convert gray scale isn't in use because of the combine version
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
                    || image[x][y - 1] == 0 || image[x][y + 1] == 0))
                  || ((image[x - 1][y] == 255 && image[x - 2][y] == 0
                       && image[x + 1][y] == 255 && image[x + 2][y] == 0
                       && image[x][y - 1] == 255 && image[x][y - 2] == 0
                       && image[x][y + 1] == 255 && image[x][y + 2] == 0)))) {
                temp_image[x][y] = 255;
            } else {
                temp_image[x][y] = 0;
            }
        }
    }

    //Using memcpy to copy full bitmap instead of iterating through the whole bitmap
    // memcpy(eroded_image, temp_image, BMP_WIDTH * BMP_HEIGTH);
    for (int x = 0; x < BMP_WIDTH; x++) {
        for (int y = 0; y < BMP_HEIGTH; y++) {
            eroded_image[x][y] = temp_image[x][y];
        }
    }
}

void draw_cross() {
    struct PositionNode *ptr = headCross;
    while (ptr != NULL) {
        for (int n = -8; n <= 8; n++) {
            for (int k = -1; k <= 1; k++) {
                if (!(ptr->x + n < 0 || ptr->x + n >= BMP_WIDTH || ptr->y + k < 0 || ptr->y + k >= BMP_HEIGTH)) {
                    original_image[ptr->x + n][ptr->y + k][0] = 255;
                    original_image[ptr->x + n][ptr->y + k][1] = 0;
                    original_image[ptr->x + n][ptr->y + k][2] = 0;

                    original_image[ptr->x + k][ptr->y + n][0] = 255;
                    original_image[ptr->x + k][ptr->y + n][1] = 0;
                    original_image[ptr->x + k][ptr->y + n][2] = 0;
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
                    if (x + n < 0 || x + n >= BMP_WIDTH) {
                        image[x + n][y] = 0;
                    }
                    if (y + n < 0 || y + n >= BMP_HEIGTH) {
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
                            if (!(x + n < 0 || x + n >= BMP_WIDTH || y + k < 0 || y + k >= BMP_HEIGTH)) {
                                image[x + n][y + k] = 0;
                            }
                        }
                    }
                    cell_count++;
                    if (BENCHMARK) {
                        total_cell_count++;
                    }
                    insert(x, y);
                    //add position to list
                    //add  1 to count
                }
            }
        }
    }
}


// Partly inspired by https://c-for-dummies.com/blog/?p=3246
void get_all_test_cases(char input_directory[], char output_directory[]) {
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
            char input_path[256] = {0};
            char output_path[256] = {0};

            // Append 
            strcat(input_path, input_directory);
            strcat(input_path, "\\");
            strcat(input_path, entry->d_name);
            strcat(output_path, output_directory);
            strcat(output_path, "\\");
            strcat(output_path, entry->d_name);

            char *dot = strrchr(input_path, '.');
            if (dot && (strcmp(dot, ".bmp") == 0)) {
                insert_paths(input_path, output_path);
            }
            // Recursively get files
            get_all_test_cases(input_path, output_directory);
        }
    }
    closedir(folder);
}


// Run the count cells algorithm
void count_cells(char input_file[], char output_file[]) {
    reset_position_list();

    //Load image from file
    read_bitmap(input_file, original_image);

    // Convert image to grayscale
    //convert_to_grayscale(original_image);
    // Apply binary threshold to image
    //apply_binary_threshold(gray_image);
    //Convverting to grayscale and applying binary threshold in one function
    //removes 2 forloops
    convert_to_grayscale_and_apply_binary_threshold(original_image);

    cell_count = 0;
    erode_image_recursive(gray_image);
    draw_cross();

    if (PRINT_POSITIONS) {
        print_cell_positions();
    }


    //! Save image to file
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
        printf("Cell count = %d\n", cell_count);
        return;
    }
    erode_image_recursive(eroded_image);
}

void print_cell_positions() {
    printf("Position of cells (x, y):");
    struct PositionNode *ptr = headCross;
    while (ptr != NULL) {
        printf(" (%d, %d)", ptr->x, ptr->y);
        ptr = ptr->next;
    }
    printf("\n");
}



//Main function
int main(int argc, char **argv) {
    //argc counts how may arguments are passed
    //argv[0] is a string with the name of the program
    //argv[1] is the first command line argument (input image)
    //argv[2] is the second command line argument (output image)

    // Running all test cases requires the CLA's to be folders
    // Running specific test case requires the CLA's to be bmp files

    printf("Beginning!\n");


    //Checking that 2 arguments are passed
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <output file path> <output file path>\n", argv[0]);
        exit(1);
    }

    // Run all or specific test
    if (RUN_ALL) {
        get_all_test_cases(argv[1], argv[2]);
        run_all_test_cases();
    } else {
        count_cells(argv[1], argv[2]);
    }
    printf("Done!\n");

    return 0;
}

void run_all_test_cases() {
    // Iterate over the paths in the linked list
    printf("Running all samples\n");
    struct pathNode *current = head;

    if (BENCHMARK) {
        clock_t start = clock();
        while (current != NULL) {
            printf("Running sample: %s\n", current->input_path);
            count_cells(current->input_path, current->output_path);
            reset_position_list();
            current = current->next;
        }
        clock_t end = clock();
        double time_spent = (double) (end - start) / CLOCKS_PER_SEC;
        printf("Time spent: %f\n", time_spent);
        printf("Total cell count: %d\n", total_cell_count);
    } else {
        while (current != NULL) {
            printf("Running sample: %s\n", current->input_path);
            count_cells(current->input_path, current->output_path);
            reset_position_list();
            current = current->next;
        }
    }

    return;

}
