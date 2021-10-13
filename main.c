/*
 * CS 261: Main driver
 *
 * Name: Noah Kaiser
 */

#include "p1-check.h"
#include "p2-load.h"
#include "p3-disas.h"
#include "p4-interp.h"

int main(int argc, char ** argv) {
  // parse command-line options
  bool print_header = false;
  bool print_segment = false;
  bool print_membrief = false;
  bool print_memfull = false;
  bool print_disas_code = false;
  bool print_diasas_data = false;
  bool print_ex_normal = false;
  bool print_ex_trace = false;
  char * fn = NULL;

  // validate command line parsing

  
  if (!parse_command_line_p4(argc, argv, &print_header, &print_segment, &print_membrief, & print_memfull, &print_disas_code, &print_diasas_data, &print_ex_normal, &print_ex_trace, &fn)) {
    exit(EXIT_FAILURE);
  }

  if (fn != NULL) {

    // open Mini-ELF binary
    FILE * f = fopen(fn, "r");

    if (!f) {
      printf("Failed to read file\n");
      exit(EXIT_FAILURE);
    }

    // P1: load and check Mini-ELF header
    elf_hdr_t header;

    if (!read_header(f, & header)) {
      printf("Failed to read file\n");
      exit(EXIT_FAILURE);
    }

    // P1 output
    if (print_header) {
      dump_header(header);
    }

    // check if file contains valid program headers
    elf_phdr_t phdr[header.e_num_phdr];
    for (int i = 0; i < header.e_num_phdr; i++) {
      if (!read_phdr(f, header.e_phdr_start + i * 20, & phdr[i])) {
        printf("Failed to read file\n");
        exit(EXIT_FAILURE);
      }
    }

    // once valid, read the program headers into memory
    fseek(f, header.e_phdr_start, SEEK_SET);
    for (int i = 0; i < header.e_num_phdr; i++) {
      fread( & phdr[i], sizeof(elf_phdr_t), 1, f);
    }

    if (print_segment) {
      printf(" Segment   Offset    VirtAddr  FileSize  Type      Flag\n");
      for (int i = 0; i < header.e_num_phdr; i++) {
        printf("  %02d", i);
        printf("       0x%04x", phdr[i].p_offset);
        printf("    0x%04x", phdr[i].p_vaddr);
        printf("    0x%04x", phdr[i].p_filesz);

        if (phdr[i].p_type == 0) {
          printf("    DATA");
        }
        if (phdr[i].p_type == 1) {
          printf("    CODE");
        }
        if (phdr[i].p_type == 2) {
          printf("    STACK");
        }
        if (phdr[i].p_type == 3) {
          printf("    HEAP");
        }
        if (phdr[i].p_type == 4) {
          printf("    UNKNOWN");
        }

        if (phdr[i].p_flag == 4) {
          printf("      R  ");
        }
        if (phdr[i].p_flag == 5) {
          printf("      R X");
        }
        if (phdr[i].p_type == 2 && phdr[i].p_flag == 6) {
          printf("     RW ");
        } else if (phdr[i].p_flag == 6) {
          printf("      RW ");
        }

        printf("\n");
      }
    }

    // check that the segment in memory is valid so that virtual memory can be read
    byte_t * mem = (byte_t * ) malloc(MEMSIZE * sizeof(byte_t));
    for (int i = 0; i < header.e_num_phdr; i++) {
      if (!load_segment(f, mem, phdr[i])) {
        printf("Failed to read file\n");
        exit(EXIT_FAILURE);
      }
    }
    // print full memory segmment
    if (print_memfull) {
      dump_memory(mem, 0, MEMSIZE);
    }

    // print brief memory segment from program header
    if (print_membrief) {
      for (int i = 0; i < header.e_num_phdr; i++) {
        dump_memory(mem, phdr[i].p_vaddr, phdr[i].p_vaddr + phdr[i].p_filesz);
      }
    }
    // dissassemble contents of memory
    if (print_disas_code) {
        printf("Disassembly of executable contents:\n");
        for (int i = 0; i < header.e_num_phdr; i++) {
            if (phdr[i].p_type == 0x0001) {
            disassemble_code(mem, &phdr[i], &header);
            }
        }
    }

    y86_t cpu; //y86 cpu
    y86_reg_t valA, valE; //y86 register values
    bool cnd; //y86 cpu conditional state
    int count = 0; //store number of cpu executions

    //initialize the cpu registers and set the program counter
    memset(&cpu, 0, sizeof(y86_t)); 
    cpu.stat = AOK;
    cpu.pc = header.e_entry;

    // decode and a execute a set of cpu instructions
    if (print_ex_normal) {
          while (cpu.stat == AOK) {
              y86_inst_t ins = fetch(&cpu, mem);
              if (cpu.stat == AOK) { 
                  valE = decode_execute(&cpu, ins, &cnd, &valA);
                  memory_wb_pc(&cpu, ins, mem, cnd, valA, valE);
                  count++;
              }
          }
          printf("Beginning execution at 0x%04x\n", header.e_entry);
          dump_cpu_state(cpu);
          printf("Total execution count: %d\n", count);
      }

    /* fetch, decode and and execute a set of cpu instructions
       and dissasemble the entire cpu state 
    */
    if (print_ex_trace) {
        printf("Beginning execution at 0x%04x\n", header.e_entry);
        dump_cpu_state(cpu);
        while (cpu.stat == AOK) {
            y86_inst_t ins = fetch(&cpu, mem); 
            if (cpu.stat == AOK) {
                valE = decode_execute(&cpu, ins, &cnd, &valA);
                memory_wb_pc(&cpu, ins, mem, cnd, valA, valE);
                printf("\nExecuting: ");
                disassemble(ins);
                printf("\n");
                count++;
            } else { //log when the instruction passed is invalid
                printf("\n");
                printf("Invalid instruction at 0x%04lx\n", cpu.pc);
            }
            dump_cpu_state(cpu);
        }
        printf("Total execution count: %d\n\n", count);
        dump_memory(mem, 0, MEMSIZE);
    }

    // cleanup
    free(mem);
    fclose(f);

  }

  return EXIT_SUCCESS;
}