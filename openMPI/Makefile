SRC_ORIG = lap_orig.c
SRC_N4 = mpi_lap_n4.c
SRC_N8 = mpi_lap_n8.c
SRC_N16 = mpi_lap_n16.c

CC = gcc
MPICC = mpicc
RUN = mpirun

CFLAGS = -Wall 
CFLAGS += -std=c99
CFLAGS += -D_BSD_SOURCE
LDFLAGS = -lm

TARGET = mpi_orig
TARGET += mpi_n4
TARGET += mpi_n8
TARGET += mpi_n16

all: $(TARGET)

mpi_orig: $(SRC_ORIG)
	$(CC) -Wall -o $@ $(SRC_ORIG) $(LDFLAGS)

mpi_n4: $(SRC_N4)
	$(MPICC) $(CFLAGS) -o $@ $(SRC_N4) $(LDFLAGS)

mpi_n8: $(SRC_N8)
	$(MPICC) $(CFLAGS) -o $@ $(SRC_N8) $(LDFLAGS)

mpi_n16: $(SRC_N16)
	$(MPICC) $(CFLAGS) -o $@ $(SRC_N16) $(LDFLAGS)

run_orig:
	@echo "==== Run orginal laplace_serial ====  "
	./mpi_orig

run_4:
	@echo "==== Run mpi laplace_serial with node 4 ===="
	$(RUN) -np 4 ./mpi_n4

run_8:
	@echo "==== Run mpi laplace_serial with node 8 ===="
	$(RUN) -np 8 ./mpi_n8

run_16:
	@echo "==== Run mpi laplace_serial with node 16 ===="
	$(RUN) -np 16 ./mpi_n16

clean:
	rm -f $(TARGET)
