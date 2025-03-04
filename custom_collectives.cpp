#include <iostream> 
#include <mpi.h>

#include "custom_collectives.h"

int custom_many2many(int *send_data, int *sendcounts, int** recv_data_ptr, int rank, int size) {
  // Calculate send displacements
  int *send_displs = new int[size];
  send_displs[0] = 0;
  for (int i = 1; i < size; i++) {
      send_displs[i] = send_displs[i-1] + sendcounts[i-1];
  }
  
  // Exchange sendcounts to determine recvcounts
  int *recvcounts = new int[size];
  
  for (int i = 0; i < size; i++) {
      if (i == rank) {
          recvcounts[i] = sendcounts[i];  // Data sent to self
          continue;
      }
      
      // Arrange communication to avoid deadlocks
      if (rank < i) {
          // Lower rank sends first, then receives
          MPI_Send(&sendcounts[i], 1, MPI_INT, i, rank, MPI_COMM_WORLD);
          MPI_Recv(&recvcounts[i], 1, MPI_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      } else {
          // Higher rank receives first, then sends
          MPI_Recv(&recvcounts[i], 1, MPI_INT, i, i, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Send(&sendcounts[i], 1, MPI_INT, i, rank, MPI_COMM_WORLD);
      }
  }
  
  // Calculate total receive size and receive displacements
  int total_recv_size = 0;
  int *recv_displs = new int[size];
  recv_displs[0] = 0;
  for (int i = 0; i < size; i++) {
      total_recv_size += recvcounts[i];
      if (i < size - 1) {
          recv_displs[i+1] = recv_displs[i] + recvcounts[i];
      }
  }
  
  // Allocate memory for receive buffer
  int *recv_data = new int[total_recv_size];
  *recv_data_ptr = recv_data;
  
  // Exchange the actual data using the same pattern
  for (int i = 0; i < size; i++) {
      if (i == rank) {
          // Copy data to self
          for (int j = 0; j < sendcounts[i]; j++) {
              recv_data[recv_displs[i] + j] = send_data[send_displs[i] + j];
          }
          continue;
      }
      
      // Arrange communication to avoid deadlocks
      if (rank < i) {
          // Lower rank sends first, then receives
          MPI_Send(&send_data[send_displs[i]], sendcounts[i], MPI_INT, i, rank + size, MPI_COMM_WORLD);
          MPI_Recv(&recv_data[recv_displs[i]], recvcounts[i], MPI_INT, i, i + size, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      } else {
          // Higher rank receives first, then sends
          MPI_Recv(&recv_data[recv_displs[i]], recvcounts[i], MPI_INT, i, i + size, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Send(&send_data[send_displs[i]], sendcounts[i], MPI_INT, i, rank + size, MPI_COMM_WORLD);
      }
  }
  
  // Clean up
  delete[] send_displs;
  delete[] recvcounts;
  delete[] recv_displs;
  
  return total_recv_size;
}

void custom_allreduce_sum(int *local, int *global, int num_elem, int rank, int size) {
  // First, copy local data to global buffer
  for (int i = 0; i < num_elem; i++) {
      global[i] = local[i];
  }
  
  // Temporary buffer for receiving data
  int *temp_buf = new int[num_elem];
  
  // Recursive doubling algorithm following hypercube pattern
  for (int step = 1; step < size; step *= 2) {
      int partner = rank ^ step; // XOR to find communication partner in hypercube
      
      // Skip if partner is outside valid range
      if (partner >= size) continue;
      
      if (rank < partner) {
          // Lower rank sends first, then receives
          MPI_Send(global, num_elem, MPI_INT, partner, 0, MPI_COMM_WORLD);
          MPI_Recv(temp_buf, num_elem, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      } else {
          // Higher rank receives first, then sends
          MPI_Recv(temp_buf, num_elem, MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
          MPI_Send(global, num_elem, MPI_INT, partner, 0, MPI_COMM_WORLD);
      }
      
      // Perform reduction (SUM)
      for (int i = 0; i < num_elem; i++) {
          global[i] += temp_buf[i];
      }
  }
  
  // Free temporary buffer
  delete[] temp_buf;
}