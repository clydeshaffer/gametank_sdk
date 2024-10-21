import numpy as np
import librosa
import scipy.fftpack as fft
import argparse
import struct
import os

# Set up argument parser
parser = argparse.ArgumentParser(description='Process a WAV file and save the eight most dominant frequencies per 16ms frame as 16-bit integers.')
parser.add_argument('file', type=str, help='Path to the WAV file')
parser.add_argument('--output', type=str, help='Output binary file')
parser.add_argument('--threshold', type=float, default=0.01, help='Amplitude threshold for frequencies (default: 0.01)')
parser.add_argument('--weight_factor', type=float, default=0.2, help='Weighting factor to emphasize higher frequencies (default: 0.2)')

# Parse arguments
args = parser.parse_args()
file_path = args.file

output_file_path = args.output

if output_file_path is None:
    output_file_path = os.path.splitext(file_path)[0] + '.bin'

threshold = args.threshold
weight_factor = args.weight_factor

# Load the WAV file
audio_data, sample_rate = librosa.load(file_path, sr=None)

# Define chunk size: 16ms in samples
chunk_duration_ms = 16
chunk_size = int((chunk_duration_ms / 1000) * sample_rate)

# Function to get the eight most dominant frequencies, with weighting
def get_dominant_frequencies(fft_result, sample_rate, chunk_size, num_freqs=8, threshold=0.01, weight_factor=0.2):
    # Compute the magnitudes of the FFT
    magnitudes = np.abs(fft_result[:chunk_size // 2])  # We take only half due to symmetry of FFT
    
    # Calculate the corresponding frequencies for each FFT bin
    freqs = np.fft.fftfreq(chunk_size, 1/sample_rate)[:chunk_size // 2]
    
    # Apply frequency-dependent weighting
    weighting = 1 + (freqs / max(freqs)) * weight_factor  # Weighting increases with frequency
    weighted_magnitudes = magnitudes * weighting

    # Apply threshold: discard frequencies with weighted magnitudes below the threshold
    valid_indices = np.where(weighted_magnitudes >= threshold)[0]

    # If we don't have enough valid frequencies, fill the rest with zeros
    if len(valid_indices) < num_freqs:
        dominant_indices = np.argsort(weighted_magnitudes[valid_indices])[-num_freqs:]
        dominant_freqs = np.zeros(num_freqs)
        dominant_freqs[:len(valid_indices)] = valid_indices * (sample_rate / chunk_size)
    else:
        # Get indices of the eight largest weighted magnitudes above the threshold
        dominant_indices = np.argsort(weighted_magnitudes[valid_indices])[-num_freqs:]
        dominant_freqs = valid_indices[dominant_indices] * (sample_rate / chunk_size)

    # Return the frequencies sorted by magnitude
    return dominant_freqs

# Prepare to write to the output binary file
with open(output_file_path, 'wb') as f_out:
    # Process each 16ms chunk
    for i in range(0, len(audio_data), chunk_size):
        chunk = audio_data[i:i+chunk_size]
        
        # If chunk is shorter than chunk_size (last chunk), pad with zeros
        if len(chunk) < chunk_size:
            chunk = np.pad(chunk, (0, chunk_size - len(chunk)), mode='constant')
        
        # Apply FFT to the chunk
        fft_result = fft.fft(chunk)
        
        # Get the eight most dominant frequencies with thresholding and frequency weighting
        dominant_freqs = get_dominant_frequencies(fft_result, sample_rate, chunk_size, threshold=threshold, weight_factor=weight_factor)
        
        # Convert frequencies to 16-bit integers
        dominant_freqs_int = np.round(dominant_freqs * 4.68600695639).astype(np.int16)
        
        # Write the 8 frequencies as 16-bit integers to the binary file
        f_out.write(struct.pack('8h', *dominant_freqs_int))

# Now the binary file contains the eight most dominant frequencies for each 16ms chunk
