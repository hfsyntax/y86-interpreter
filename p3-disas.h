#ifndef __CS261_P3__
#define __CS261_P3__

#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "elf.h"
#include "y86.h"

/**
 * @brief Load a Y86 instruction from memory
 *
 * @param cpu Pointer to Y86 CPU structure with the PC address to be loaded
 * @param memory Pointer to the beginning of the Y86 address space
 * @returns Populated Y86 instruction structure
 */
y86_inst_t fetch (y86_t *cpu, byte_t *memory);

/**
 * @brief Print the program usage text
 *
 * @param argv Array of command-line options
 */
void usage_p3 (char **argv);

/**
 * @brief Parse the command line options
 *
 * @param argc Number of command-line options
 * @param argv Array of command-line options
 * @param header Pointer to boolean flag for printing the file header
 * @param segments Pointer to boolean flag for printing the program segments
 * @param membrief Pointer to boolean flag for printing the memory briefly
 * @param memfull Pointer to boolean flag for printing the memory in full
 * @param disas_code Pointer to boolean flag for disassembling code segments
 * @param disas_data Pointer to boolean flag for disassembling data segments
 * @param filename Pointer to string buffer for Mini-ELF filename
 * @returns True if the command-line options were valid, false if not
 */
bool parse_command_line_p3 (int argc, char **argv,
        bool *header, bool *segments, bool *membrief, bool *memfull,
        bool *disas_code, bool *disas_data, char **filename);

/**
 * @brief Print the disassembly of a Y86 instruction to standard out
 *
 * @param inst Y86 instruction structure to be printed
 */
void disassemble (y86_inst_t inst);

/**
 * @brief Print the disassembly of a Y86 code segment
 *
 * @param memory Pointer to the beginning of the Y86 address space
 * @param phdr Program header of segment to be printed
 * @param hdr File header (needed to detect the entry point)
 */
void disassemble_code   (byte_t *memory, elf_phdr_t *phdr, elf_hdr_t *hdr);

/**
 * @brief Print the disassembly of a Y86 read/write data segment
 *
 * @param memory Pointer to the beginning of the Y86 address space
 * @param phdr Program header of segment to be printed
 */
void disassemble_data   (byte_t *memory, elf_phdr_t *phdr);

/**
 * @brief Print the disassembly of a Y86 read-only data segment
 *
 * @param memory Pointer to the beginning of the Y86 address space
 * @param phdr Program header of segment to be printed
 */
void disassemble_rodata (byte_t *memory, elf_phdr_t *phdr);

#endif
