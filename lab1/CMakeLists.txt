cmake_minimum_required(VERSION 3.20)
project(lab1 C)

set(CMAKE_C_STANDARD 17)

add_executable(lab1
        monte_carlo.c
        mandelbrot.c
        main_mandelbrot.c
        main_monte_carlo.c
)
