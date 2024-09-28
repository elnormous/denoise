#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>
#include <rnnoise.h>

int main(int argc, char* argv[])
{
    const char* input_file = NULL;
    const char* output_file = NULL;
    SF_INFO sfinfo;
    SNDFILE* infile = NULL;
    SNDFILE* outfile = NULL;
    short* buffer = NULL;
    float* x = NULL;
    DenoiseState* st = NULL;
    int i;
    sf_count_t frames_read = 0;
    int rnn_frame_size = 0;
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
        fprintf(stderr, "Error opening input file\n");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    if (sfinfo.channels != 1)
    {
        fprintf(stderr, "The input file must be mono\n");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    st = rnnoise_create(NULL);
    if (!st)
    {
        fprintf(stderr, "Failed to create rnnoise state\n");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    rnn_frame_size = rnnoise_get_frame_size();
    buffer = malloc((unsigned long)rnn_frame_size * sizeof(short));
    x = malloc((unsigned long)rnn_frame_size * sizeof(float));

    printf("Opening output file\n");
    outfile = sf_open(output_file, SFM_WRITE, &sfinfo);

    if (!outfile)
    {
        fprintf(stderr, "Error opening output file\n");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    printf("Processing data\n");

    for (;;)
    {
        memset(buffer, 0, (unsigned long)rnn_frame_size * sizeof(short));
        frames_read = sf_read_short(infile, buffer, rnn_frame_size);

        for (i = 0; i < rnn_frame_size; ++i)
            x[i] = (float)buffer[i];

        rnnoise_process_frame(st, x, x);

        for (i = 0; i < rnn_frame_size; ++i)
            buffer[i] = (short)roundf(x[i]);

        sf_write_short(outfile, buffer, frames_read);

        if (frames_read < rnn_frame_size)
            break;
    }

cleanup:
    rnnoise_destroy(st);
    free(x);
    free(buffer);
    sf_close(outfile);
    sf_close(infile);

    printf("WAV file processed successfully!\n");

    return result;
}

