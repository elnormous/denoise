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
    float gain = 1.0f;
    char overwrite = 'n';
    int result = EXIT_SUCCESS;

    while ((opt = getopt(argc, argv, "i:o:s:g:y")) != -1)
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
                if (strength < 0.0F || strength > 1.0F)
                {
                    fprintf(stderr, "Strength must be between 0.0 and 1.0\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'g':
                gain = strtof(optarg, NULL);
                if (gain < 0.0F)
                {
                    fprintf(stderr, "Gain must be positive\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'y':
                overwrite = 'y';
                break;
            default:
                fprintf(stderr, "Usage: %s -i <input_file> -o <output_file> [-s strength] [-g gain] [-y]\n", argv[0]);
                return EXIT_FAILURE;
        }

    if (input_file == NULL || output_file == NULL)
    {
        fprintf(stderr, "Input and output files are required.\n");
        fprintf(stderr, "Usage: %s -i <input_file> -o <output_file> [-s strength] [-g gain] [-y]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (access(input_file, F_OK) != 0)
    {
        fprintf(stderr, "Input file does exist.");
        return EXIT_FAILURE;
    }

    if (overwrite != 'y' && access(output_file, F_OK) == 0)
    {
        fprintf(stderr, "Output file exists. Overwrite? [y/n] ");
        overwrite = (char)getc(stdin);

        if (overwrite != 'y')
            return EXIT_SUCCESS;
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

    if (sfinfo.samplerate != 48000)
        fprintf(stderr, "For best performance resample the file to 48kHz\n");

    st = rnnoise_create(NULL);
    if (!st)
    {
        fprintf(stderr, "Failed to create rnnoise state\n");
        result = EXIT_FAILURE;
        goto cleanup;
    }

    rnn_frame_size = rnnoise_get_frame_size();
    buffer = malloc((size_t)rnn_frame_size * sizeof(short));
    x = malloc((size_t)rnn_frame_size * sizeof(float));

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
        memset(buffer, 0, (size_t)rnn_frame_size * sizeof(short));
        frames_read = sf_read_short(infile, buffer, rnn_frame_size);

        for (i = 0; i < rnn_frame_size; ++i)
            x[i] = (float)buffer[i];

        rnnoise_process_frame(st, x, x);

        for (i = 0; i < rnn_frame_size; ++i)
            buffer[i] = (short)roundf((strength * x[i] + (1.0f - strength) * (float)buffer[i]) * gain);

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

