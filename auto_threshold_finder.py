import pandas as pd
import numpy as np
import glob
import os

# Path to the SA04 folder containing all .txt files
folder_path = "SisFall_dataset/SisFall_dataset/SA04/*.txt"

# Initialize storage variables
max_adl = 0.0      # Highest peak acceleration of Daily Activities (starts with 'D')
min_fall = 9999.0  # Lowest peak acceleration of Falls (starts with 'F')

# Get a list of all files in the folder
file_list = glob.glob(folder_path)
print(f"Automatically scanning and processing {len(file_list)} files...\n")

for filepath in file_list:
    filename = os.path.basename(filepath)
    
    try:
        # Read data quickly, no need to plot
        df = pd.read_csv(filepath, sep=',', header=None, usecols=[0, 1, 2], names=['X', 'Y', 'Z'])
        df['Total'] = np.sqrt(df['X']**2 + df['Y']**2 + df['Z']**2)
        
        peak_val = df['Total'].max()
        
        # Identify Daily Activity files (starts with 'D')
        if filename.startswith('D'):
            if peak_val > max_adl:
                max_adl = peak_val
                
        # Identify Fall files (starts with 'F')
        elif filename.startswith('F'):
            if peak_val < min_fall:
                min_fall = peak_val
                
    except Exception as e:
        # Skip if there's a malformed file
        pass

# Print summary report
print("-" * 50)
print("SISFALL AUTOMATED ANALYSIS REPORT (Subject SA04)")
print("-" * 50)
print(f"1. Strongest Daily Activity (No fall) peak acceleration: {max_adl:.2f}")
print(f"2. Lightest Fall peak acceleration: {min_fall:.2f}")

if min_fall > max_adl:
    # Calculate the midpoint (average) to use as the threshold
    threshold = (max_adl + min_fall) / 2
    print(f"\n=> RECOMMENDED OPTIMAL SENSITIVITY THRESHOLD: {threshold:.2f}")
else:
    print("\n[WARNING] Data overlap detected! The peak of a normal activity is higher than a fall.")
    print("You need to add a 'free fall' (Min Valley) detection algorithm to your C++ code.")
