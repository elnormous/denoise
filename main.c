#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sndfile.h>
#include <rnnoise.h>

int main(int argc, char* argv[])
{
    int opt;
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
    float strength = 1.0f;
    int result = EXIT_SUCCESS;

    while ((opt = getopt(argc, argv, "i:o:s:")) != -1)
        switch (opt)
        {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 's':
                strength = strtof(optarg, NULL);
                break;
            default:
                fprintf(stderr, "Usage: %s -i <input_file> -o <output_file> [-s strength]\n", argv[0]);
                return 1;
        }

    if (input_file == NULL || output_file == NULL)
    {
        fprintf(stderr, "Error: Input and output files are required.\n");
        fprintf(stderr, "Usage: %s -i <input_file> -o <output_file> [-s strength]\n", argv[0]);
        return EXIT_FAILURE;
    }

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
            buffer[i] = (short)roundf(strength * x[i] + (1.0f - strength) * (float)buffer[i]);

        sf_write_short(outfile, buffer, frames_read);

        if (frames_read < rnn_frame_size)
            break;
    }

    printf("WAV file processed successfully!\n");

cleanup:
    rnnoise_destroy(st);
    free(x);
    free(buffer);
    sf_close(outfile);
    sf_close(infile);

    return result;
}

