
# Denoise

`denoise` is a simple command-line tool that removes noise from audio files using the RNNoise library. This tool processes input audio files and generates denoised output files, making it easier to clean up noisy recordings.

## Features
- Denoises audio files using the RNNoise library.
- Supports custom noise suppression strength.
- Supports a variety of audio formats via `sndfile`.

## Requirements
- `sndfile` (to read and write audio files)
- `RNNoise` (for noise suppression)

## Installation

### Prerequisites

Before building, make sure you have installed the following dependencies:
- `libsndfile` 
- `rnnoise`

On a Debian-based system, you can install these dependencies with:
```bash
sudo apt-get install libsndfile1-dev librnnoise-dev
```

### Build Instructions

To build `denoise`, simply clone the repository and run `make` in the root directory:

```bash
git clone https://github.com/yourusername/denoise.git
cd denoise
make
```

## Usage

The `denoise` tool can be used from the command line with the following syntax:

```bash
./denoise -i <input_file> -o <output_file> [-s <strength>] [-g <gain>] [-y]
```

- `-i <input_file>`: The input audio file that you want to denoise.
- `-o <output_file>`: The output file where the denoised audio will be saved.
- `-s <strength>`: (Optional) Set the denoising strength value. Defaults to 1.0 if not provided.
- `-g <gain>`: (Optional) Set the gain level of the output. Defaults to 1.0.
- `-y`: (Optional) If the output file exists, it will be overwritten.

### Example

To denoise an audio file with the default strength:
```bash
./denoise noisy_audio.wav clean_audio.wav
```

To specify a custom strength:
```bash
./denoise noisy_audio.wav clean_audio.wav 0.8
```

## License

This is free and unencumbered software released into the public domain. See the [UNLICENSE](UNLICENSE) file for details.

## Contributing

Pull requests and issues are welcome! Please feel free to contribute.

---

For more information on RNNoise, visit the [RNNoise GitHub repository](https://github.com/xiph/rnnoise).
