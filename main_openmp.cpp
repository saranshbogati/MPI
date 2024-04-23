#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define MAX_GRID_SIZE 16384
#define MAX_ITERATIONS 200

char current_grid[MAX_GRID_SIZE][MAX_GRID_SIZE];
char next_grid[MAX_GRID_SIZE][MAX_GRID_SIZE];

int width, height;

void initialize_grid(int grid_width, int grid_height) {
    width = grid_width;
    height = grid_height;
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            current_grid[j][i] = 0;
        }
    }
    for (int i = 0; i < width; i++) current_grid[0][i] = 1;
    for (int i = 0; i < height; i++) current_grid[i][0] = 1;
}

int count_neighbors(int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                count += current_grid[ny][nx];
            }
        }
    }
    return count;
}

int update_grid() {
    int live_count = 0;
    #pragma omp parallel for reduction(+:live_count) collapse(2)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int neighbors = count_neighbors(x, y);
            next_grid[y][x] = (current_grid[y][x] == 1 && (neighbors == 2 || neighbors == 3)) || (current_grid[y][x] == 0 && neighbors == 3) ? 1 : 0;
            live_count += next_grid[y][x];
        }
    }

    #pragma omp parallel for collapse(2)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            current_grid[y][x] = next_grid[y][x];
        }
    }
    return live_count;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: ./open width height\n");
        exit(0);
    }

    int grid_width = atoi(argv[1]);
    int grid_height = atoi(argv[2]);

    initialize_grid(grid_width, grid_height);

    double start_time = omp_get_wtime();
    int initial_count = update_grid();
    int should_continue = 1, iteration = 0, population_count = initial_count;

    while (iteration < MAX_ITERATIONS-1 && should_continue) {
        population_count = update_grid();
        printf("iter = %d, Global population count = %d\n", iteration, population_count);
        if (population_count == 0 || population_count < initial_count / 50 || population_count > 50 * initial_count) {
            should_continue = 0;
        }
        iteration++;
    }

    double end_time = omp_get_wtime();
    printf("OpenMP Time taken: %f seconds\n", end_time - start_time);

    return 0;
}
