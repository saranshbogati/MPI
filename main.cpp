#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define MAX_SIZE 16384
#define DEFAULT_ITERATIONS 200


char grid[MAX_SIZE][MAX_SIZE];
char next_grid[MAX_SIZE][MAX_SIZE];

int grid_width, grid_height;

void initialize_grid(int width, int height) {
    grid_width = width;
    grid_height = height;
    for (int i = 0; i < grid_width; i++) {
        for (int j = 0; j < grid_height; j++) {
            grid[j][i] = 0;
        }
    }
    for (int i = 0; i < grid_width; i++) grid[0][i] = 1;
    for (int i = 0; i < grid_height; i++) grid[i][0] = 1;
}

int count_neighbors(int x, int y, int start_row, int end_row, char* top_row, char* bottom_row, int grid_width, int rank, int size) {
    int count = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < grid_width) {
                if (ny < start_row) {
                    if (rank != 0) count += top_row[nx];
                } else if (ny >= end_row) {
                    if (rank != size - 1) count += bottom_row[nx];
                } else {
                    count += grid[ny][nx];
                }
            }
        }
    }
    return count;
}

void exchange_boundary(int rank, int size, int start_row, int end_row, char* top_row, char* bottom_row, int grid_width) {
    int above = rank - 1, below = rank + 1;
    if (rank == 0) above = MPI_PROC_NULL;
    if (rank == size - 1) below = MPI_PROC_NULL;

    MPI_Sendrecv(grid[start_row], grid_width, MPI_CHAR, above, 0,
                 bottom_row, grid_width, MPI_CHAR, below, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Sendrecv(grid[end_row - 1], grid_width, MPI_CHAR, below, 1,
                 top_row, grid_width, MPI_CHAR, above, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void update_grid(int start_row, int end_row, int grid_width) {
    for (int y = start_row; y < end_row; y++) {
        for (int x = 0; x < grid_width; x++) {
            grid[y][x] = next_grid[y][x];
        }
    }
}

void simulate_life(int rank, int size, int init_count) {
    int local_height = grid_height / size + (rank == size - 1 ? grid_height % size : 0);
    int start_row = rank * (grid_height / size);
    int end_row = start_row + local_height;
    char top_row[MAX_SIZE], bottom_row[MAX_SIZE];

    printf("Process %d handling rows %d to %d.\n", rank, start_row, end_row - 1);
    int iter = 0;
    int global_count = init_count;
    int continue_simulation = 1;
    // iter < DEFAULT_ITERATIONS && (global_count < 50 * init_count && global_count > init_count / 50

    while (iter < DEFAULT_ITERATIONS && continue_simulation) {
        // printf("Iter < default %s", iter<DEFAULT_ITERATIONS)
        // printf("INIT COUNT %s", iter<DEFAULT_ITERATIONS)
        // printf("Iter < default %s", iter<DEFAULT_ITERATIONS)
        // std::cout << "Iter < default " << iter<DEFAULT_ITERATIONS << endl;
        // std::cout << "Iter < default " << iter<DEFAULT_ITERATIONS << endl;
        // std::cout << "Iter < default " << iter<DEFAULT_ITERATIONS << endl;
        // std::cout << "Iter < default " << iter<DEFAULT_ITERATIONS << endl;
        exchange_boundary(rank, size, start_row, end_row, top_row, bottom_row, grid_width);

        int local_count = 0;
        for (int y = start_row; y < end_row; y++) {
            for (int x = 0; x < grid_width; x++) {
                int neighbors = count_neighbors(x, y, start_row, end_row, 
                                                (y == start_row) ? top_row : NULL, 
                                                (y == end_row - 1) ? bottom_row : NULL, 
                                                grid_width, rank, size);
                next_grid[y][x] = (grid[y][x] == 1 && (neighbors == 2 || neighbors == 3)) || (grid[y][x] == 0 && neighbors == 3) ? 1 : 0;
                local_count += next_grid[y][x];
            }
        }

        update_grid(start_row, end_row, grid_width);

        int global_count;
        MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
        if (rank == 0) {
            printf("iter = %d, Global population count = %d\n", iter, global_count);
            if (global_count == 0 || iter >= DEFAULT_ITERATIONS || global_count < init_count / 50 || global_count > 50 * init_count) {
                continue_simulation = 0; 
            }
        }
                MPI_Bcast(&continue_simulation, 1, MPI_INT, 0, MPI_COMM_WORLD);

        iter++;
    }
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    double start, end;
    start = MPI_Wtime();

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 3) {
        if (rank == 0) printf("Usage: mpirun -np <num_processes> ./main width height\n");
        MPI_Finalize();
        exit(0);
    }

    int grid_width = atoi(argv[1]);
    int grid_height = atoi(argv[2]);
    // printf("Argc count is %d", argc);
    int iterations = grid_width + grid_height;

    
    initialize_grid(grid_width, grid_height);
    
    int local_init_count = 0;
    for (int y = 0; y < grid_height; y++) {
        for (int x = 0; x < grid_width; x++) {
            local_init_count += grid[y][x];
        }
    }

    int init_count;

    MPI_Reduce(&local_init_count, &init_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    // printf("Init count is %d", init_count);
    if (rank == 0) {
        printf("Initial global population count: %d\n", init_count);
    }

    simulate_life(rank, size, init_count);
    end = MPI_Wtime();
    MPI_Finalize();
    
    if (rank == 0) { 
        printf("MPI Time: %f seconds\n", end - start);
    }

    return 0;
}
