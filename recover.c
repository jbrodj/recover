#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// **NOTES**
// JPEG signature is a four byte sequence
// The first three are always: 0xff 0xd8 0xff -- ie. 255 216 255
// The fourth byte is something starting with 0xe[0-f]
// (ie. that byte's first four bits are always 1110, or 224-239 inclusive)
// Memory card is formatted using FAT with block size of 512 bytes.
// and is block-aligned (ie. an image will only start at the beginning
// of one of these blocks). Slack space in this case shouldn't impact
// viewing of the jpgs because the card was zeroed before use.
// What do we want to do here?
// --Accept 1 command line arg, the raw data to be parsed.
// --Handle errors for cases where incorrect command line args, or unreadable data is provided
// (error 1 expected)
// --Iterate over the memory card in chunks of 512B
// --When the start of a block matches the JPEG signature,
//   open a new file and write that data to it (possibly across multiple
//   blocks of 512B), until we encounter another signature.
// --Close current file, and open new one for the next image.
// --Files generated should be named sequentially starting with 000.jpg.

int main(int argc, char *argv[])
{
    // Check for correct num of command line args
    if (argc != 3)
    {
        printf("Usage: `./recover infile block_size`\n");
        return 1;
    }

  // Check that block size arg is numeric.
  for (int l = 0, length = strlen(argv[2]); l < length; l++ )
  {
    char *block_arg = argv[2];
    if (isdigit(block_arg[l]) == 0)
    {
      printf("Block size must be a number\n");
      return 1;
    }
  }

    long block_size = strtol(argv[2], 0, 10);

    // Open raw data and check check that it is readable
    FILE *input = fopen(argv[1], "r");

    if (input == NULL)
    {
        printf("Data not readable\n");
        return 1;
    }

    // Create buffer
    uint8_t block[block_size];

    // Initialize file counter
    int file_num = 0;

    // Initialize output file pointer
    FILE *output = NULL;

    // Declare filename variable for our current jpg
    char *filename = malloc(8);

    // While there is file left to read (ie while there is a 512B chunk to read).
    // `== 1` here means == 1 x sizeof(block) (ie. 1 x 512B).
    while (fread(block, sizeof(block), 1, input) == 1)
    {
        // Check for JPEG signature
        if (block[0] == 0xFF && block[1] == 0xD8 && block[2] == 0xFF &&
            (block[3] >= 0xE0 && block[3] <= 0xEF))
        {

            // Close any open output file
            if (output != NULL)
            {
                fclose(output);
            }

            // Generate name for new file using the file num
            sprintf(filename, "%03i", file_num);
            filename = strcat(filename, ".jpg");

            // Open new file and iterate file counter
            output = fopen(filename, "w");
            file_num++;
        }

        // While file is open, and new jpg signature is not detected, write to the file.
        if (output != NULL)
        {
            fwrite(block, sizeof(block), 1, output);
        }
    }

    printf("Found %i files\n", file_num);

    // Deallocate memory and end program.
    if (output != NULL)
    {
        fclose(output);
    }
    free(filename);
    fclose(input);
    return 0;
}
