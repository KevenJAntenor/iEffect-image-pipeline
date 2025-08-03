# 💥📱✨ iEffect: Accelerated Image Processing Pipeline

## Project Overview
iEffect is a high-performance image processing application capable of applying a variety of effects to large sets of images efficiently. By utilizing a multi-threaded pipeline structure, the application enhances processing speed significantly.

## 🪜 Pipeline Structure
The processing pipeline consists of three stages, operating in a FIFO queue manner:

Each stage is capable of running multiple threads, optimizing throughput and resource management.

## ✨ Features
- Multi-threaded processing for load, effect application, and save stages.
- FIFO queues to manage task order and memory consumption.
- Semaphore mechanisms to regulate queue sizes and ensure stability.
- Modular pipeline code, reusable for different contexts.

## Getting Started
The application's pipeline structure is defined in `pipeline.c`, with data structures in `pipeline.h`, including:
- `struct pipeline`: Defines the treatment stages.
- `struct pipeline_stage`: Manages the semaphores, queue, and thread information for each stage.
- `struct work_item`: Represents the work unit, carrying image information through the pipeline.

## Execution Guide
To execute iEffect, configure the environment and run the application as follows:

bash
# Set up the environment for the executable path
source env.sh

# Process a single image with the default settings
ieffect --input tests/cat.png --output cat-serial.png

# Process a single image using a 4-thread pipeline
ieffect --input tests/cat.png --output cat-serial.png -m -n 4

# Performance testing for serial versus pipeline modes (download images first if needed)
./data/fetch.sh
time ieffect --input data/ --output results/
time ieffect --input data/ --output results/ -m -n 4

## ⚙️ Compilation

apt-get install libpng-dev

cmake -S . -B build
cd build
make
make test
