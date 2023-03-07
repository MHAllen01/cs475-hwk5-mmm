#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include "rtclock.h"
#include "mmm.h"

int main(int argc, char *argv[]) {
	double clockstart, clockend;
	double clockstartSeq, clockendSeq, clockstartPar, clockendPar;

	if (strcmp(argv[0], "./mmm") == 0 && argv[1] != NULL) {
		// Valid run command

		if (strcmp(argv[1], "S") == 0 && argv[3] == NULL) {
			// Run sequential
			if (argv[2] != NULL) {
				char *check;
				size = strtod(argv[2], &check);
				
				// Valid size
				if (*check == '\0' || check != argv[2]) {
					// Store size as a global variable
				
					// Run the program 4 times
					mmm_init();
					// Run the sequential program
					clockstart = rtclock();
					mmm_seq();
					clockend = rtclock(); // stop clocking
					// Reset the matrices
					mmm_reset(inputMatrix1);
					mmm_reset(inputMatrix2);
					mmm_reset(outputMatrixSeq);

					for (int i=0; i<3; i++) {
						// Initialize and populate the two matrices
						mmm_init();
						// Run the sequential program
						clockstart += rtclock();
						mmm_seq();
						clockend += rtclock(); // stop clocking
						// Reset the matrices
						mmm_reset(inputMatrix1);
						mmm_reset(inputMatrix2);
						mmm_reset(outputMatrixSeq);
					}
					
					// Free the matrices
					mmm_freeup();

					// Keep track of the average time
					double avgTime = ((clockend - clockstart) / 3.0);

					printf("\n========\n");
					printf("mode: sequential\n");
					printf("thread count: 1\n");
					printf("size: %d\n", size);
					printf("========\n");
					printf("Sequential Time (avg of 3 runs): %.6f sec\n", avgTime);
				} else {
					// Error
					printf("\n%s is not a valid size.\n", argv[2]);
				}
			} else {
				// Error
				printf("\nNot enough parameters. Try ./mmm S <size of matrices>\n");
			}

		} else if (strcmp(argv[1], "P") == 0) {
			// Run parallel
			if (argv[2] != NULL && argv[3] != NULL && argv[4] == NULL) {
				char *check;
				numThreads = strtod(argv[2], &check);
				
				// Valid number of threads
				if (*check == '\0' || check != argv[3]) {
					size = strtod(argv[2], &check);
					// Valid size
					if (*check == '\0' || check != argv[2]) {
						size = strtod(argv[3], &check);

						// Prepare thread arguments
						thread_args *args = (thread_args*) malloc(sizeof(thread_args) * numThreads);

						for (int i=0; i<numThreads; i++){
							args[i].tid = i;
							args[i].start = i * size / numThreads + 1;
							args[i].end = (i + 1) * size / numThreads;
						}

						// // Allocate space to hold threads
						pthread_t *threads = (pthread_t*) malloc(sizeof(pthread_t) * numThreads);
						
						// Initialize the matrices
						mmm_init(); 

						// Used to calculate the verification of the matrices
						double difference = 0.0;

						// Start Parallel clock
						clockstartPar = rtclock();
						for (int i=0; i<numThreads; i++) {
							// Create the threads
							pthread_create(&threads[i], NULL, mmm_par, &args[i]);
						}

						// Join the threads
						for (int i=0; i<numThreads; i++) {
							// Join the threads
							pthread_join(threads[i], NULL);
						}

						// End Parallel clock
						clockendPar = rtclock();

						// Run the sequential version
						clockstartSeq = rtclock();
						mmm_seq();
						clockendSeq = rtclock();

						// Compare the two outputs
						difference += mmm_verify();

						// Reset all arrays
						mmm_reset(inputMatrix1);
						mmm_reset(inputMatrix2);
						mmm_reset(outputMatrixSeq);
						mmm_reset(outputMatrixPar);

							// Run each 3 times and compare
							for (int i=0; i<3; i++) {
								// Initialize the matrices
								mmm_init(); 


								// Start Parallel clock
								clockstartPar += rtclock();
								for (int i=0; i<numThreads; i++) {
									// Create the threads
									pthread_create(&threads[i], NULL, mmm_par, &args[i]);
								}

								// Join the threads
								for (int i=0; i<numThreads; i++) {
									// Join the threads
									pthread_join(threads[i], NULL);
								}

								// End Parallel clock
								clockendPar += rtclock();

								// Run the sequential version
								clockstartSeq += rtclock();
								mmm_seq();
								clockendSeq += rtclock();

								// Compare the two outputs
								difference += mmm_verify();

								// Reset all arrays
								mmm_reset(inputMatrix1);
								mmm_reset(inputMatrix2);
								mmm_reset(outputMatrixSeq);
								mmm_reset(outputMatrixPar);
						}

						// Free the matrices
						mmm_freeup();

						// Free threads array
						free(threads);
						threads = NULL;
						free(args);
						args = NULL;


						// Used for time calculations
						double avgTimeSeq = (clockendSeq - clockstartSeq) / 3.0;
						double avgTimePar = (clockendPar - clockstartPar) / 3.0;
						double speedup = avgTimeSeq / avgTimePar;

						// Print outputs
						printf("\n========\n");
						printf("mode: parallel\n");
						printf("thread count: %d\n", numThreads);
						printf("size: %d\n", size);
						printf("========\n");
						printf("Sequential Time (avg of 3 runs): %.6f sec\n", avgTimeSeq);
						printf("Parallel Time (avg of 3 runs): %.6f sec\n", avgTimePar);
						printf("Speedup: %.6f\n", speedup);
						printf("Verifying... largest error between parallel and sequential matrix: %.6f\n", difference);

					} else {
						// Error
						printf("\n%s is not a valid size.\n", argv[2]);
					}

				} else {
					// Error
					printf("\n%s is not a valid number of threads.\n", argv[2]);
				}
			} else {
				// Error
				printf("\nNot enough parameters. Try ./mmm P <# of threads> <size of matrices>\n");
			}
		}
	} else {
		printf("\nNot enough parameters. Try ./mmm <P|S>\n");
	}
	return 0;
}
