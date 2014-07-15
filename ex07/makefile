#MakeFile for a serial FWave Solver
#G++ OTIONS
GO = -c -fmessage-length=0 -O3 -mtune=native -fstrict-aliasing -fargument-noalias -DNDEBUG -DWAVE_PROPAGATION_SOLVER=1
GO2 = -c -fmessage-length=0 -O3 -mtune=native -fstrict-aliasing -fargument-noalias -DNDEBUG -DWAVE_PROPAGATION_SOLVER=2
GO3 = -c -fmessage-length=0 -O3 -mtune=native -fstrict-aliasing -fargument-noalias -DNDEBUG -DWAVE_PROPAGATION_SOLVER=0

fwave:
	g++ -o build/SWE_WavePropagationBlock.o $(GO) -Isrc/include src/blocks/SWE_WavePropagationBlock.cpp
	g++ -o build/SWE_Block.o $(GO) -Isrc/include src/blocks/SWE_Block.cpp
	g++ -o build/Logger.o $(GO) -Isrc/include src/tools/Logger.cpp
	g++ -o build/VtkWriter.o $(GO) -Isrc/include src/writer/VtkWriter.cpp
	g++ -o build/swe_simple.o $(GO) -Isrc/include src/examples/swe_simple.cpp
	g++ -o build/fwaveSolver build/SWE_WavePropagationBlock.o build/SWE_Block.o build/Logger.o build/VtkWriter.o build/swe_simple.o

augrie:
	g++ -o build/SWE_WavePropagationBlock.o $(GO2) -Isrc/include src/blocks/SWE_WavePropagationBlock.cpp
	g++ -o build/SWE_Block.o $(GO2) -Isrc/include src/blocks/SWE_Block.cpp
	g++ -o build/Logger.o $(GO2) -Isrc/include src/tools/Logger.cpp
	g++ -o build/VtkWriter.o $(GO2) -Isrc/include src/writer/VtkWriter.cpp
	g++ -o build/swe_simple.o $(GO2) -Isrc/include src/examples/swe_simple.cpp
	g++ -o build/augrieSolver build/SWE_WavePropagationBlock.o build/SWE_Block.o build/Logger.o build/VtkWriter.o build/swe_simple.o

hybrid:

	g++ -o build/SWE_WavePropagationBlock.o $(GO3) -Isrc/include src/blocks/SWE_WavePropagationBlock.cpp
	g++ -o build/SWE_Block.o $(GO3) -Isrc/include src/blocks/SWE_Block.cpp
	g++ -o build/Logger.o $(GO3) -Isrc/include src/tools/Logger.cpp
	g++ -o build/VtkWriter.o $(GO3) -Isrc/include src/writer/VtkWriter.cpp
	g++ -o build/swe_simple.o $(GO3) -Isrc/include src/examples/swe_simple.cpp
	g++ -o build/hybridsolver build/SWE_WavePropagationBlock.o build/SWE_Block.o build/Logger.o build/VtkWriter.o build/swe_simple.o
#END