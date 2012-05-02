OBJ= main.o VGSA.o FFT.o FrFFT.o
CPL= g++ -c
LNK= g++ -o

main : $(OBJ)
	$(LNK) main $(OBJ) -lfftw3 -lm

main.o : main.cpp calibration.h surfaces.h
	$(CPL) main.cpp

VGSA.o : ../../transform_model/src/VGSA.cpp
	$(CPL) ../../transform_model/src/VGSA.cpp

FFT.o : ../../transform_model/src/FFT.cpp 
	$(CPL) ../../transform_model/src/FFT.cpp 

FrFFT.o : ../../transform_model/src/FrFFT.cpp 
	$(CPL) ../../transform_model/src/FrFFT.cpp
  
.PHONY : clean

clean :
	rm main $(OBJ)

run :
	./main
