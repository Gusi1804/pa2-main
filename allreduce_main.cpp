#include <iostream>
#include <mpi.h>

#include "custom_collectives.h"

#include <cassert>
#include <fstream>

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  /* initialize the variables needed */
  int  num_elem;
  int* local_array;
  int* global_sum_custom;
  int* global_sum_default = nullptr;

  /* read the input file PE 0 and send all the processors the data they need */
  if (argc < 2 || argc > 3) {
    if (rank == 0) std::cout << "Usage: " << argv[0] << " <input> (OPTIONAL: <output.txt>)" << std::endl;
    MPI_Finalize();
    return 1;
  }

  int LEN = 0;
  std::ifstream file(argv[1]);
  if (!file.is_open()) {
  std::cerr << "Error: could not open file " << argv[1] << std::endl;
  MPI_Finalize();
  return 1;
}

  if (rank == 0) {
    /* read the first line into a variable file_num_pes */
    int file_num_pes;
    file >> file_num_pes;
    assert(file_num_pes == size);

    /* read the second line into a variable called length */
    file >> LEN;
  }

  MPI_Bcast(&LEN, 1, MPI_INT, 0, MPI_COMM_WORLD);
  local_array = new int[LEN];
  global_sum_custom = new int[LEN];
  num_elem = LEN;

  if (rank == 0) {
    /* fill up ther local_array variable of PE 0 */
    for (int i = 0; i < LEN; ++i) file >> local_array[i];

    int* tmp = new int[LEN];
    for (int pe = 1; pe < size; ++pe) {
      /* each line is command seperated integers, put them all in temp */
      for (int i = 0; i < LEN; ++i) file >> tmp[i];
      MPI_Send(tmp, LEN, MPI_INT, pe, 0, MPI_COMM_WORLD);
    }
    delete[] tmp;
  } else {
    MPI_Recv(local_array, LEN, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  /* call the custom function for the same */
  double start2 = MPI_Wtime();
  custom_allreduce_sum(local_array, global_sum_custom, num_elem, rank, size);
  double end2 = MPI_Wtime();
  double local_custom_time = end2 - start2, global_custom_time;
  MPI_Allreduce(&local_custom_time, &global_custom_time, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
  if (rank == 0) printf("Time taken by custom function: %f\n", global_custom_time);

  /* call the default MPI function */
  double start_default = MPI_Wtime();
  global_sum_default = new int[num_elem];
  MPI_Allreduce(local_array, global_sum_default, num_elem, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  double end_default = MPI_Wtime();
  double local_default_time = end_default - start_default, global_default_time;
  MPI_Allreduce(&local_default_time, &global_default_time, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
  if (rank == 0) printf("Time taken by default function: %f\n", global_default_time);

  /* Verify that both implementations produce the same results */
  bool results_match = true;
  for (int i = 0; i < num_elem; i++) {
    if (global_sum_custom[i] != global_sum_default[i]) {
      results_match = false;
      if (rank == 0) {
        printf("Results mismatch at index %d: custom=%d, default=%d\n", 
              i, global_sum_custom[i], global_sum_default[i]);
      }
      break;
    }
  }

  if (rank == 0 && results_match) {
    printf("Custom implementation results match default MPI implementation!\n");
  }

  if (argc == 2) {
    MPI_Finalize();
    return 0;
  }

  /* Write the output of the custom_all_reduce_sum to output file */
  if (rank == 0) {
    std::ofstream out(argv[2]);
    for (int i = 0; i < LEN; ++i) out << global_sum_custom[i] << " ";
    out << std::endl;
    out.close();
  }

  /* free all dynamically allocated memory */
  delete[] local_array;
  delete[] global_sum_custom;
  delete[] global_sum_default;

  /* MPI Finalize */
  MPI_Finalize();
  return 0;
}
