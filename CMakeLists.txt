cmake_minimum_required(VERSION 3.22)
project(RISC_V_Simulator)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/src)

add_executable(code
        main.cpp
        src/ALU.cpp
        src/BranchPredictor.cpp
        src/CPU.cpp
        src/Clock.cpp
        src/Decoder.cpp
        src/InstructionUnit.cpp
        src/LoadStoreBuffer.cpp
        src/Memory.cpp
        src/RegisterFile.cpp
        src/ReorderBuffer.cpp
        src/ReservationStation.cpp
        src/config.cpp
        src/utils/WriteController.cpp
)

add_executable(test
        test.cpp
)