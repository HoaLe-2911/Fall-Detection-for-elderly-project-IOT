import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

# Path to the data file you are opening in VS Code
file_path = "SisFall_dataset/SisFall_dataset/SA04/F01_SA04_R01.txt" 

def plot_sisfall_data(filepath):
    # Read the data file, skip any bad lines
    try:
        # SisFall dataset has multiple columns. The first 3 columns are X, Y, Z accelerometer axes.
        df = pd.read_csv(filepath, sep=',', header=None, usecols=[0, 1, 2], names=['X', 'Y', 'Z'])
        
        # Calculate the Total Acceleration (Vector Magnitude) - similar to the sqrt function in Arduino
        df['Total_Acceleration'] = np.sqrt(df['X']**2 + df['Y']**2 + df['Z']**2)

        # Create the plot
        plt.figure(figsize=(12, 6))
        
        # Plot individual axes (Optional: You can comment these out with '#' if it looks too messy)
        plt.plot(df.index, df['X'], label='X Axis', alpha=0.5)
        plt.plot(df.index, df['Y'], label='Y Axis', alpha=0.5)
        plt.plot(df.index, df['Z'], label='Z Axis', alpha=0.5)
        
        # Plot the Total Acceleration line (The most important line to find the fall threshold)
        plt.plot(df.index, df['Total_Acceleration'], label='Total Acceleration', color='red', linewidth=2)

        plt.title(f"Acceleration Graph - File: {filepath.split('/')[-1]}")
        plt.xlabel("Samples (Time)")
        plt.ylabel("Acceleration Magnitude")
        plt.legend()
        plt.grid(True)
        plt.show()
        
        # Print the peak value to help determine the Sensitivity threshold
        max_accel = df['Total_Acceleration'].max()
        print(f"-> The peak acceleration in this file is: {max_accel:.2f}")

    except Exception as e:
        print(f"Error reading file: {e}")

# Execute the function
plot_sisfall_data(file_path)
