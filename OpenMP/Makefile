build: parabrutfor.c
	gcc -o parabrutfor parabrutfor.c sha256.c -fopenmp

run: build
	export OMP_NUM_THREADS=4 && time ./parabrutfor

clean:
	rm -f parabrutfor
