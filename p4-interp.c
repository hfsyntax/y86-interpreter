/*
 * CS 261 PA4: Mini-ELF interpreter
 *
 * Name: Noa Kaiser
 */

#include "p4-interp.h"

/**********************************************************************
 *                         REQUIRED FUNCTIONS
 *********************************************************************/

// decode the sepcified y86 condtion code and peform it's execution stages  
y86_reg_t decode_execute (y86_t *cpu, y86_inst_t inst, bool *cnd, y86_reg_t *valA)
{
    // intialize each register value
    y86_reg_t valE = 0;
    y86_reg_t valB = 0;
    y86_reg_t valC = 0;

    // check if the condition code is valid
    if (cnd == NULL) {
        cpu->stat = INS;
        return valE;
    }
    // iterrate over the specified condition code
    switch (inst.icode) {
        case HALT:
            cpu->stat = HLT;
            valE = 0;
            break;
        case NOP:
            break;
        case CMOV:
            *valA = cpu->reg[inst.ra];
            valE = cpu->reg[inst.ra];
            switch (inst.ifun.cmov) {
                case RRMOVQ:
                    *cnd = true;
                    break;
                case CMOVLE:
                    *cnd = (cpu->sf ^ cpu->of) | cpu->zf;
                    break;
                case CMOVL:
                    *cnd = cpu->sf ^ cpu->of;
                    break;
                case CMOVE:
                    *cnd = cpu->zf;
                    break;
                case CMOVNE:
                    *cnd = !cpu->zf;
                    break;
                case CMOVGE:
                    *cnd = !(cpu->sf ^ cpu->of);
                    break;
                case CMOVG:
                    *cnd = !(cpu->sf ^ cpu->of) & !cpu->zf;
                    break;
                case BADCMOV:
                    *cnd = false;
                    cpu->stat = INS;
                    break;
                default:
                    *cnd = false;
                    cpu->stat = INS;
                    break;
            }
            break;
        case IRMOVQ:
            valE = inst.valC.v;
            break;
        case RMMOVQ:
            *valA = cpu->reg[inst.ra];
            valB = cpu->reg[inst.rb];
            valE = valB + inst.valC.d;
            break;
        case MRMOVQ:
            valB = cpu->reg[inst.rb];
            valC = inst.valC.d;
            valE = valB + valC;
            if (valE < 0 || valE > MEMSIZE)
            {
                cpu->stat = ADR;
            }
            break;
        case OPQ:
            valB = cpu->reg[inst.rb];
            *valA = cpu->reg[inst.ra];
            switch (inst.ifun.op)
            {  
                case ADD:
                    valE = valB + *valA;
                    break;
                case SUB:
                    valE = valB - *valA;
                    break;
                case AND:
                    valE = *valA & valB;
                    break;
                case XOR:
                    valE = *valA ^ valB;
                    break;
                case BADOP:
                    cpu->stat = INS;
                    break;
                default:
                    cpu->stat = INS;
                    break;
            }

            if (valE == 0) {
                cpu->sf = false;
                cpu->zf = true;
            } else if (valE & 0x8000000000000000) {
                cpu->sf = true;
                cpu->zf = false;
            } else 
                cpu->sf = false;
            if (valE != ((int)valE)) 
               cpu->of = true;
            else 
                cpu->of = false;
            break;
        case JUMP:
            switch (inst.ifun.jump) {
                case JMP:
                    *cnd = true;
                    break;
                case JLE:
                    *cnd = (cpu->sf ^ cpu->of) | cpu->zf;
                    break;
                case JL:
                    *cnd = (cpu->of ^ cpu->sf);
                    break;
                case JE:
                    *cnd = cpu->zf;
                    break;
                case JNE:
                    *cnd = !cpu->zf;
                    break;
                case JGE:
                    *cnd = !(cpu->sf ^ cpu->of);
                    break;
                case JG:
                    *cnd = !(cpu->of ^ cpu->sf) & !cpu->zf;
                    break;
                case BADJUMP:
                    cpu->stat = INS;
                    break;
                default:
                    cpu->stat = INS;
                    break;
            }
            break;
        case CALL:
            if ((inst.valC.dest < 0) || (inst.valC.dest > MEMSIZE)) 
                cpu->stat = ADR;
            else 
                valE = cpu->reg[RSP] - 8;
            break;
        case RET:
            valB = cpu->reg[RSP];
            *valA = cpu->reg[RSP];
            valE = cpu->reg[RSP] + 8;
            break;
        case PUSHQ:
            *valA = cpu->reg[inst.ra];
            valE = cpu->reg[RSP] - 8;
            if (valE >= MEMSIZE || valE < 0)
                cpu->stat = ADR;
            break;
        case POPQ:
            *valA = cpu->reg[RSP];
            valE = cpu->reg[RSP] + 8;
            if (valE < 0 || valE > MEMSIZE) 
                cpu->stat = ADR;
            break;
        default:
            cpu->stat = INS;
            break;
    }

    return valE;
}

// read and write memory back to the cpu depending on the y86 condition code
void memory_wb_pc (y86_t *cpu, y86_inst_t inst, byte_t *memory,
        bool cnd, y86_reg_t valA, y86_reg_t valE)
{
    // iterrate over the specified condition code
    switch (inst.icode) {
        case HALT:
            cpu->pc = 0;
            cpu->zf = 0;
            cpu->sf = 0;
            cpu->of = 0;
            break;
        case NOP:
            cpu->pc = inst.valP;
            break;
        case CMOV:
            if (cnd == true) {
                cpu->reg[inst.rb] = valE;
            }
            cpu->pc = inst.valP;
            break;
        case IRMOVQ:
            cpu->reg[inst.rb] = valE;
            cpu->pc = inst.valP;
            break;
        case RMMOVQ:
            *(y86_reg_t*)&memory[valE] = valA;
            cpu->pc = inst.valP;
            break;
        case MRMOVQ:
            if (valE + 8 >= MEMSIZE || valE < 0)
            {
                cpu->stat = ADR;
            } else {
                cpu->reg[inst.ra] = *(y86_reg_t*)&memory[valE];
            }
            cpu->pc = inst.valP;
            break;
        case OPQ:
            cpu->reg[inst.rb] = valE;
            cpu->pc = inst.valP;
            break;
        case JUMP:
            if (cnd == false) {
                cpu->pc = inst.valP;
            } else {
                cpu->pc = inst.valC.dest;
            }
            break;
        case CALL:
            if (cpu->reg[RSP] == 0) {
                cpu->stat = ADR;
                cnd = true;
            } else {
                *(y86_reg_t*)&memory[valE] = inst.valP;
                cpu->reg[RSP] = valE;
            }
            cpu->pc = inst.valC.dest;
            break;
        case RET:
            if (cpu->reg[RSP] >= MEMSIZE || cpu->reg[RSP] <= 0) {
                cpu->stat = ADR;
                cnd = true;
            } else {
                cpu->reg[RSP] = valE;
            }
            cpu->pc= *(y86_reg_t*)&memory[valA];
            break;
        case PUSHQ:
            if (cpu->reg[RSP] >= MEMSIZE || cpu->reg[RSP] <= 0) {
                cnd = true;
                cpu->stat = ADR;
            } else {
                *(y86_reg_t*)&memory[valE] = valA;
                cpu->reg[RSP] = valE;
            }
            cpu->pc = inst.valP;
            break;
        case POPQ:
            if (cpu->reg[RSP] >= MEMSIZE || cpu->reg[RSP] < 0) {
                cnd = true;
                cpu->stat = ADR;
            } else {
                cpu->reg[inst.ra] = *(y86_reg_t*)&memory[valA];
                cpu->reg[inst.ra] = *(y86_reg_t*)&memory[valA];
                cpu->reg[RSP] = valE;
            }
            cpu->pc = inst.valP;
            break;
        case INVALID:
            break;
        default:
            break;
    }
}

/**********************************************************************
 *                         OPTIONAL FUNCTIONS
 *********************************************************************/

void usage_p4 (char **argv)
{
    printf("Usage: %s <option(s)> mini-elf-file\n", argv[0]);
    printf(" Options are:\n");
    printf("  -h      Display usage\n");
    printf("  -H      Show the Mini-ELF header\n");
    printf("  -a      Show all with brief memory\n");
    printf("  -f      Show all with full memory\n");
    printf("  -s      Show the program headers\n");
    printf("  -m      Show the memory contents (brief)\n");
    printf("  -M      Show the memory contents (full)\n");
    printf("  -d      Disassemble code contents\n");
    printf("  -D      Disassemble data contents\n");
    printf("  -e      Execute program\n");
    printf("  -E      Execute program (trace mode)\n");
}

bool parse_command_line_p4 (int argc, char **argv,
        bool *header, bool *segments, bool *membrief, bool *memfull,
        bool *disas_code, bool *disas_data,
        bool *exec_normal, bool *exec_trace, char **filename)
{
   
   
    if (argv == NULL || header == NULL || filename == NULL) {
        return false;
    }

  int invalid_f1 = 0;
  int invalid_f2 = 0;

  // parameter parsing w/ getopt()
  int c;
  while ((c = getopt(argc, argv, "hHsmfaMdDeE")) != -1) {
    switch (c) {
    case 'h':
      usage_p4(argv);
      return true;
    case 'H':
      *header = true;
      break;
    case 's':
      *segments = true;
      break;
    case 'm':
      *membrief = true;
      invalid_f1++;
      invalid_f2++;
      break;
    case 'M':
      *memfull = true;
      invalid_f1++;
      break;
    case 'f':
      *header = true;
      * segments = true;
      * memfull = true;
      invalid_f2++;
      break;
    case 'a':
      *header = true;
      * segments = true;
      * membrief = true;
      break;
    case 'd':
        *disas_code = true;
        break;
    case 'D':
        *disas_data = true;
        break;
    case 'e':
        *exec_normal = true;
        break;
    case 'E':
        *exec_trace = true;
        break;
    default:
      usage_p4(argv);
      return false;
    }
  }

  /*  check for invalid flag combos
      - m & M
      - m & f
  */
  if (invalid_f1 > 1 || invalid_f2 > 1) {
    usage_p4(argv);
    return false;
  }

  if (optind != argc - 1) {
    // no filename (or extraneous input)
    usage_p4(argv);
    return false;
  }

  // check if both execution flags are passed at once
  if (*exec_normal && *exec_trace) {
    usage_p4(argv);
    return false;
  }

  * filename = argv[optind]; // save filename

  return true;
}
// print the status of the y86 cpu condition codes
void dump_cpu_state (y86_t cpu)
{
    printf("Y86 CPU state:\n");
    printf("  %%rip: %016lx   flags: Z%d S%d O%d     ", cpu.pc, cpu.zf, cpu.sf, cpu.of);

    switch (cpu.stat) {
        case AOK:
            printf("AOK\n");
            break;
        case HLT:
            printf("HLT\n");
            break;
        case ADR:
            printf("ADR\n");
            break;
        case INS:
            printf("INS\n");
            break;
        default:
            break;
    }
    
    // print the contents of the cpu's registers
    printf("  %%rax: %016lx    %%rcx: %016lx\n", cpu.reg[RAX], cpu.reg[RCX]);
    printf("  %%rdx: %016lx    %%rbx: %016lx\n", cpu.reg[RDX], cpu.reg[RBX]);
    printf("  %%rsp: %016lx    %%rbp: %016lx\n", cpu.reg[RSP], cpu.reg[RBP]);
    printf("  %%rsi: %016lx    %%rdi: %016lx\n", cpu.reg[RSI], cpu.reg[RDI]);
    printf("   %%r8: %016lx     %%r9: %016lx\n", cpu.reg[R8], cpu.reg[R9]);
    printf("  %%r10: %016lx    %%r11: %016lx\n", cpu.reg[R10], cpu.reg[R11]);
    printf("  %%r12: %016lx    %%r13: %016lx\n", cpu.reg[R12], cpu.reg[R13]);
    printf("  %%r14: %016lx\n", cpu.reg[R14]);
}
