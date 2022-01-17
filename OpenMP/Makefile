build: parabrutfor.c
	gcc -o parabrutfor parabrutfor.c -fopenmp -lssl -lcrypto

run: build
	export OMP_NUM_THREADS=64 && time ./parabrutfor

clean:
	rm -f parabrutfor
