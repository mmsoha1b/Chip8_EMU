#include<string>
using namespace std;
class Chip8                             // Choosing data types based on memory each data type takes Since they're all basically integers anyway
{
    unsigned short opcode;              // Each opcode is 2 bytes long
    unsigned char memory[4096];         // 4096 Memory locations (4k). Each memory location 1byte(8bits)
    unsigned char V[16];                // 15 gen puspose Registers [V0-VE]. VF for carry flag
    unsigned short I;                   // Index registe    r
    unsigned short pc;                  // Program counter
    unsigned char gfx[64*32];           // For graphics. Chop8 draws on 64pix*32pix screen
    unsigned char delay_timer;          // Delay timer Count up to 60 Hz
    unsigned char sound_timer;          // Sound Timer Beep when = 0

    unsigned short stack[16];           // Stack of 16 levels
    unsigned short sp;                  // Stack Pointer
    unsigned char key[16];              // HEX Keyboard
    unsigned char chip8_font[80];       // FontSet(???????)
public:
    Chip8();
    void init();
    void one_cycle();
    void load(string);
    void cls_scrn();                  
};