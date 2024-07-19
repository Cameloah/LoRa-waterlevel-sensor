import numpy as np
from matplotlib import pyplot as plt
from scipy.ndimage import rotate
import random


pattern = np.array([[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
                    [2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2],
                    [4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4],
                    [2, 2, 2, 2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2],
                    [0, 0, 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0],
                    [0, 0, 0, 0, 0, 0, 1, 4, 1, 0, 0, 0, 0, 0, 0]])


# generate fake data
testing_data = np.zeros((18, 18))

distance_sensor_obstacle = 80
compartment_1_level = 80#random.randint(50, 220)
compartment_2_level = 80#random.randint(90, 220)
compartment_3_level = random.randint(90, 220)

compartment_1_level = max(compartment_1_level, 80)
compartment_2_level = max(compartment_2_level, 80)
compartment_3_level = max(compartment_3_level, 80)


for i in range(testing_data.shape[0]):
    for j in range(testing_data.shape[1]):
        if i < testing_data.shape[0]/2 - 1:
            testing_data[i, j] = compartment_1_level
        elif i < testing_data.shape[0]/2:
            testing_data[i, j] = distance_sensor_obstacle
        elif j < testing_data.shape[1]/2 - 1:
            testing_data[i, j] = compartment_2_level
        elif j < testing_data.shape[1]/2:
            testing_data[i, j] = distance_sensor_obstacle
        elif i < testing_data.shape[1]:
            testing_data[i, j] = compartment_3_level


# Rotate data by a random angle
angle = random.randint(0, 360)
testing_data = rotate(testing_data, angle, reshape=False, order=0)

# Crop data to the center 12x12 matrix
start_i = (testing_data.shape[0] - 10) // 2
start_j = (testing_data.shape[1] - 10) // 2

testing_data = testing_data[start_i:start_i + 10, start_j:start_j + 10]





# Take measurement: Extract the 8x8 cropped section
max_start_index = testing_data.shape[0] - 8
start_i = np.random.randint(0, max_start_index + 1)
start_j = np.random.randint(0, max_start_index + 1)

sensor_data = testing_data[start_i:start_i + 8, start_j:start_j + 8]

# Add 5% noise to the sensor data
noise_level = 0.10
noise = np.random.normal(0, noise_level, sensor_data.shape)
sensor_data = sensor_data + noise


print("sensor_data = np.array(")
print((sensor_data * 10).astype(int).tolist())
print(")")


normalized_data = (sensor_data - np.min(sensor_data)) / (np.max(sensor_data) - np.min(sensor_data))
normalized_data[normalized_data > 0.05] = 1
normalized_data = np.abs(normalized_data - 1)


kernel_ridge = np.array([[ -1, -1,  -1],
                         [ -1,  8,  -1],
                         [ -1, -1,  -1]])

kernel_edge = np.array([[ 0, -1,  0],
                        [-1,  4, -1],
                        [ 0, -1,  0]])

kernel_sharpen = np.array([[ 0, -1,  0],
                        [-1,  5, -1],
                        [ 0, -1,  0]])


def convolution(edgedetection_input, kernel):
    output = np.zeros_like(edgedetection_input)
    for i in range(edgedetection_input.shape[0]):
        for j in range(edgedetection_input.shape[1]):
            convolute_result = 0
            for m in range(kernel_ridge.shape[0]):
                for n in range(kernel_ridge.shape[1]):
                    convolute_index_k = i - kernel_ridge.shape[0] // 2 + m
                    convolute_index_l = j - kernel_ridge.shape[1] // 2 + n
                    if convolute_index_k >= 0 and convolute_index_k < edgedetection_input.shape[0] and convolute_index_l >= 0 and convolute_index_l < edgedetection_input.shape[1]:
                        convolute_result += edgedetection_input[convolute_index_k, convolute_index_l] * kernel_ridge[m, n]
            output[i, j] = convolute_result
    return output




def edge_detection_custom(edgedetection_input):
    edge_map = np.zeros_like(edgedetection_input)
    for i in range(edgedetection_input.shape[0]):
        for j in range(edgedetection_input.shape[1]):
            convolute_result = 0
            for m in range(-1, 2):
                for n in range(-1, 2):
                    if m == 0 and n == 0:
                        continue
                    convolute_index_k = i + m
                    convolute_index_l = j + n
                    if convolute_index_k >= 0 and convolute_index_k < edgedetection_input.shape[0] and convolute_index_l >= 0 and convolute_index_l < edgedetection_input.shape[1]:
                        convolute_result += abs(edgedetection_input[convolute_index_k, convolute_index_l] - edgedetection_input[i, j])
            edge_map[i, j] = convolute_result
    
    return edge_map



normalized_data = np.pad(normalized_data, ((1, 1), (1, 1)), mode='edge')

print(normalized_data.shape)

edge_map = convolution(normalized_data, kernel_edge)
edge_map = edge_map[1:-1, 1:-1]
print(edge_map.shape)

edge_map[edge_map < 0.5] = 0
edge_map[edge_map > 0.5] = 1



# Apply each Sobel kernel to the matrix and sum the results
highest_score = 0
best_kernel = None
best_angle = 0
coord_intersection = None
convolute_result = 0



patternmatching_input = edge_map



for angle in range(0, 360, 5):
    kernel = rotate(pattern, angle, reshape=False, order=1)
    for i in range(patternmatching_input.shape[0]):
        for j in range(patternmatching_input.shape[1]):
            convolute_result = 0
            for m in range(kernel.shape[0]):
                for n in range(kernel.shape[1]):
                    convolute_index_k = i - kernel.shape[0] // 2 + m
                    convolute_index_l = j - kernel.shape[1] // 2 + n
                    if convolute_index_k >= 0 and convolute_index_k < patternmatching_input.shape[0] and convolute_index_l >= 0 and convolute_index_l < patternmatching_input.shape[1]:
                        convolute_result += patternmatching_input[convolute_index_k, convolute_index_l] * kernel[m, n]
            if convolute_result > highest_score:
                highest_score = convolute_result
                best_kernel = kernel
                best_angle = angle
                coord_intersection = (i, j)



print("The intersection angle is:", best_angle, "°")
print("The highest score is:", highest_score)
print("The T-intersection is at:", coord_intersection)



x_1 = int(np.cos(np.radians(best_angle + 90)) * 4)
y_1 = int(np.sin(np.radians(best_angle + 90)) * 4)

x_2 = int(np.cos(np.radians(best_angle - 45)) * 4)
y_2 = int(np.sin(np.radians(best_angle - 45)) * 4)

x_3 = int(np.cos(np.radians(best_angle - 135)) * 4)
y_3 = int(np.sin(np.radians(best_angle - 135)) * 4)


# Check if coordinates are within the size of sensor_data
measure_at = [(coord_intersection[1] + x_1, coord_intersection[0] - y_1), 
              (coord_intersection[1] + x_2, coord_intersection[0] - y_2), 
              (coord_intersection[1] + x_3, coord_intersection[0] - y_3)]

# Adjust coordinates if they are outside the size of sensor_data
measure_at = [(max(0, min(coord[0], sensor_data.shape[1]-1)), max(0, min(coord[1], sensor_data.shape[0]-1))) for coord in measure_at]




# Define the plotting function
def plot_matrix(ax, matrix, title, coord_intersection=None, measure_at=None):
    cax = ax.imshow(matrix, cmap='gray', extent=[0, matrix.shape[0], 0, matrix.shape[1]])
    
    # Display the value of each entry inside its field
    for i in range(matrix.shape[0]):
        for j in range(matrix.shape[1]):
            ax.text(j + 0.5, matrix.shape[1] - 0.5 - i, "{:.1f}".format(matrix[i, j]), color='orange', ha='center', va='center')
    
    ax.grid(color='orange', linewidth=1)
    ax.set_xticks(np.arange(0, matrix.shape[0] + 1, 1))
    ax.set_yticks(np.arange(0, matrix.shape[1] + 1, 1))
    
    if coord_intersection:
        ax.plot(coord_intersection[1] + 0.5, matrix.shape[1] - 0.5 - coord_intersection[0], 'bs', markersize=20)
    
    if measure_at:
        for i in range(len(measure_at)):
            ax.plot(measure_at[i][0] + 0.5, matrix.shape[1] - 0.5 - measure_at[i][1], 'rs', markersize=20)
    
    ax.set_title(title)
    return cax

# Create subplots
fig, axs = plt.subplots(1, 3, figsize=(12, 6))

# Plot best_kernel without coord_intersection and measure_at
cax1 = plot_matrix(axs[0], best_kernel, "Best Kernel")

# Plot sensor_data with coord_intersection and measure_at
cax2 = plot_matrix(axs[1], sensor_data, "Sensor Data", coord_intersection, measure_at)
cax3 = plot_matrix(axs[2], edge_map, "edge_map")


# Add colorbars
fig.colorbar(cax1, ax=axs[0])
fig.colorbar(cax2, ax=axs[1])
fig.colorbar(cax3, ax=axs[2])

# Show the plot
plt.tight_layout()
plt.show()