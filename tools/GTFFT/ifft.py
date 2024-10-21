import numpy as np
import struct
import wave
import argparse
from scipy.io.wavfile import write

# Set up argument parser
parser = argparse.ArgumentParser(description='Read a binary file of dominant frequencies and generate a WAV file with sine waves.')
parser.add_argument('input', type=str, help='Input binary file with dominant frequencies')
parser.add_argument('output', type=str, help='Output WAV file')
parser.add_argument('--sample_rate', type=int, default=44100, help='Sample rate of the output WAV file')
parser.add_argument('--duration_per_frame_ms', type=int, default=16, help='Duration of each frame in milliseconds')

# Parse arguments
args = parser.parse_args()
input_file_path = args.input
output_file_path = args.output
sample_rate = args.sample_rate
duration_per_frame_ms = args.duration_per_frame_ms

# Number of samples per frame
samples_per_frame = int((duration_per_frame_ms / 1000) * sample_rate)

# Read the binary file and extract frequencies
frequencies = []
with open(input_file_path, 'rb') as f_in:
    while True:
        data = f_in.read(16)  # Read 8 16-bit integers (2 bytes each, 8*2 = 16 bytes)
        if not data:
            break
        # Unpack the 8 integers
        freqs = struct.unpack('8h', data)
        frequencies.append(freqs)

# Function to generate sine wave for a given frequency
def generate_sine_wave(frequency, duration, sample_rate):
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    wave = 0.5 * np.sin(2 * np.pi * frequency * t)  # 0.5 is the amplitude to avoid clipping
    return wave

# Initialize an empty array for the final audio signal
audio_signal = np.zeros(len(frequencies) * samples_per_frame)

# Generate sine waves for each frame and add them to the audio signal
for i, frame_freqs in enumerate(frequencies):
    frame_signal = np.zeros(samples_per_frame)
    for freq in frame_freqs:
        if freq > 0:  # Avoid generating sine waves for zero frequencies
            sine_wave = generate_sine_wave(freq / 4.68600695639, duration_per_frame_ms / 1000.0, sample_rate)
            frame_signal += sine_wave
    # Normalize the frame signal to prevent clipping
    frame_signal /= len(frame_freqs)
    
    # Insert the frame signal into the final audio signal
    audio_signal[i * samples_per_frame : (i + 1) * samples_per_frame] = frame_signal

# Ensure the audio signal is within the 16-bit range
audio_signal = np.int16(audio_signal / np.max(np.abs(audio_signal)) * 32767)

# Save the audio signal as a WAV file
write(output_file_path, sample_rate, audio_signal)

print(f"Audio saved to {output_file_path}")
