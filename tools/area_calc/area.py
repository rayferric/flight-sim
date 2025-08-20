import os
import glob
from PIL import Image
import numpy as np
import matplotlib.pyplot as plt

# Get all PNG files in current working directory and sort alphabetically
png_files = sorted(glob.glob("*.png"))

if not png_files:
    print("No PNG files found in current directory")
    exit()

areas = []
file_names = []

# Calculate area for each image
for i, filename in enumerate(png_files):
    img = Image.open(filename)
    # Convert to grayscale and normalize to 0-1 range
    gray_array = np.array(img.convert('L')) / 255.0
    
    # Count pixels with brightness > 0.5
    area = np.sum(gray_array > 0.5)
    areas.append(area)
    file_names.append(filename)

# Scale areas so maximum is 1.0
max_area = max(areas)
scaled_areas = [area / max_area for area in areas]

# Display results
for i, (filename, scaled_area) in enumerate(zip(file_names, scaled_areas)):
    print(f"{i}: {scaled_area:.6f}")

# Plot the sequence
plt.figure(figsize=(10, 6))
plt.plot(range(len(scaled_areas)), scaled_areas, 'b-o', linewidth=2, markersize=4)
plt.xlabel('Image Index')
plt.ylabel('Scaled Area')
plt.title('Scaled Area Sequence')
plt.grid(True, alpha=0.3)
plt.tight_layout()
plt.show()
