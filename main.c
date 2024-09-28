#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sndfile.h>
#include <rnnoise.h>

int main(int argc, char* argv[])
{
    const char* input_file = NULL;
    const char* output_file = NULL;
    SF_INFO sfinfo;
    SNDFILE* infile = NULL;
    SNDFILE* outfile = NULL;
    sf_count_t num_items = 0;
    short* buffer = NULL;
    float* tmp = NULL;
    DenoiseState* st = NULL;
    int i;
    int frame = 0;
    int frame_size = 0;
    int result = EXIT_SUCCESS;

    if (argc < 3)
    {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    input_file = argv[1];
    output_file = argv[2];

    printf("Reading input file\n");

    infile = sf_open(input_file, SFM_READ, &sfinfo);

    if (!infile)
    {
        printf("Error opening input file\n");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    if (sfinfo.channels != 1)
    {
        printf("The input file must be mono\n");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    // Allocate buffer for reading samples
    num_items = sfinfo.frames * sfinfo.channels;
    buffer = malloc((unsigned long)num_items * sizeof(short));

    // Read the samples
    sf_read_short(infile, buffer, num_items);

    st = rnnoise_create(NULL);
    if (!st)
    {
        printf("Failed to create rnnoise state\n");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    frame_size = rnnoise_get_frame_size();
    tmp = malloc((unsigned long)frame_size * sizeof(float));

    printf("Processing data\n");

    for (frame = 0; frame * frame_size < num_items; ++frame)
    {
        for (i = 0; i < frame_size; ++i)
            tmp[i] = frame * frame_size + i < num_items ? (float)buffer[frame * frame_size + i] : 0.0f;

        rnnoise_process_frame(st, tmp, tmp);

        for (i = 0; i < frame_size && frame * frame_size + i < num_items; ++i)
            buffer[frame * frame_size + i] = (short)roundf(tmp[i]);
    }

    printf("Writing output file\n");

    outfile = sf_open(output_file, SFM_WRITE, &sfinfo);

    if (!outfile)
    {
        printf("Error opening output file\n");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    // Write the samples to output file
    sf_write_short(outfile, buffer, num_items);

cleanup:
    rnnoise_destroy(st);
    free(tmp);
    free(buffer);
    sf_close(outfile);
    sf_close(infile);

    printf("WAV file processed successfully!\n");

    return result;
}

