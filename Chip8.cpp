#include"Chip8.h"
using namespace std;
Chip8::Chip8()
{
   init();
}
void Chip8::init()          //Resetting everything
{
    pc      =   0x200;      //System expects start of program at location 0x200
    opcode  =   0;
    I       =   0;
    sp      =   -1;
    for (int i=0;i<16;i++)
    {
        stack[i]    =   0;
    }
    for (int i=0;i<16;i++)
    {
        V[i]    =   0;
    }
    for (int i=80;i<4096;i++)
    {
        memory[i]   =  0;
    }
    for (int i=0;i<64;i++)
    {
        for(int j=0;j<32;j++)
        {
            gfx[i+j]   =  0;
        }
    }
    unsigned char chip8_fontset[80] =
    { 
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    delay_timer =   0;     //Reset timers 
    sound_timer =   0;
    for(int i =0;i<80;i++)
    {
        memory[i]=chip8_font[i];
    }

};
void Chip8::load(string s)
{
    const char* ss=s.c_str();
    FILE* game = fopen(ss,"rb");          //Check if exist. Cirrect this later
    if(game==NULL)
    {
        printf("ERROR");
        exit(0);
    }
    while(! feof(game) )
    {
        char buffer[4096];
        fgets(buffer,4096,game);
        int buffer_size=sizeof(buffer);
        for(int i=0;i<buffer_size;i++)
        {
            memory[i+512]=buffer[i];
        }
    }
};
void  Chip8::one_cycle()
{
    opcode=memory[pc]<<8|memory[pc+1];
    pc+=2;
    switch (opcode & 0xF000)                //Seperate first nibble(4 bits) of opcode
    {
        case 0x0000:
            switch (opcode & 0x000F)
            {
            case 0x000E:
                pc=stack[sp];
                stack[sp]=0;
                sp--;
            break;
            case 0x0000:
                for(int i=0;i<64;i++)
                {
                    for (int j=0;j<32;j++)
                    {
                        gfx[i+j]=0;
                    }
                }    
            break;
            default:
                printf("UNEXPECTED OXOPCODE\n");
                break;
            }
        break;

        /*case 0xA00:
            I=opcode & 0x0FFF;
        break;
        */
        case 0x1000:
            sp++;
            stack[sp]=pc;
            pc= opcode & 0x0FFF;

        break;
        case 0x2000:
            sp++;
            stack[sp]=pc;
            pc=opcode  & 0x0FFF;
        break;
        case 0x3000:
            if(V[(opcode & 0x0F00)>>8] == opcode & 0x00FF )
            {
                pc=pc+4;
            }
            else
            {
                pc+=2;
            }
        break;

        case 0x4000:
            if(V[(opcode & 0x0F00)>>8] != opcode & 0x00FF )
            {
                pc=pc+4;
            }
            else
            {
                pc+=2;
            }
        
        break;
        case 0x5000:
            if(V[(opcode & 0x0F00>>8)]==V[(opcode & 0x00F0)>>4])
            {
                pc+=4;
            }
            else
            {
                pc+=2;
            }
        break;
        case 0x6000:
            V[(opcode & 0x0F00)>>8]=opcode & 0x00FF;
            pc+=2;
        break;
        case 0x7000:
            V[(opcode & 0x0F00)>>8]+=opcode & 0x00FF;
            pc+=2;
        break;

        case 0x8000:
            switch (opcode & 0x000F)
            {
            case 0x0000:
                V[(opcode & 0x0F00>>8)]=V[(opcode & 0x00F0>>4)];
                pc+=2;
            break;
            
            case 0x0001:
                V[(opcode & 0x0F00)>>8]= V[(opcode & 0x0F00)>>8] | V[(opcode & 0x00F0)>>4];
                pc+=2;
            break;
            case 0x0002:
                V[(opcode & 0x0F00)>>8]=V[(opcode & 0x0F00)>>8] & V[(opcode & 0x00F0)>>4];
                pc+=2;
            break;
            case 0x0003:
                V[(opcode & 0x0F00)>>8]=V[(opcode & 0x0F00)>>8] ^ V[(opcode & 0x00F0)>>4];
                pc+=2;
            break;
            case 0x0004:
                if(V[(opcode & 0x0F00)>>8]+ V[(opcode & 0x00F0)>>4] > 0xFF)
                {
                    V[0xF]=1;
                }
                V[(opcode & 0x0F00)>>8]=V[(opcode & 0x0F00)>>8]+ V[(opcode & 0x00F0)>>4];
                pc+=2;

            break;
            case 0x0005://Vx-Vy
                if(V[(opcode & 0x00F0)>>4] > V[(opcode & 0x0F00)>>8])
                {
                    V[0xF]=0;
                }
                else
                {
                    V[0xF]=1;
                }
                V[(opcode & 0x0F00)>>8]=V[(opcode & 0x0F00)>>8]-V[(opcode & 0x00F0)>>4];
                pc+=2;
            break;
            case 0x0006:
                V[0xF]=V[(opcode & 0x0F00)>>8] & 0x000F & 0b0001;//Isoltae and store least sig bit
                V[(opcode & 0x0F00)>>8]=( V[(opcode & 0x0F00)>>8]) >> 1;
                pc+=2;
            break;
            case 0x0007://Vy-Vx
                if(V[(opcode & 0x00F0)>>4] >= V[(opcode & 0x0F00)>>8])
                {
                    V[0xF]=1;//No borrow
                }
                else
                {
                    V[0xF]=0;//Borrow
                }
                V[(opcode & 0x0F00)>>8]=V[(opcode & 0x00F0)>>4]-V[(opcode & 0x0F00)>>8];
                pc+=2;
            break;
            case 0x000E:
                V[0xF]=(V[(opcode & 0x0F00)>>8 ]&0xF000)>>12 & 0b1000;
                V[(opcode & 0x0F00)>>8]=V[(opcode & 0x0F00)>>8]<<1;
                pc+=2;
            break;
            default:
                printf("Unrecognize 8 OPCODE");
                break;
            }
        break;
        case 0x9000:
            if(V[(opcode & 0x0F00)>>8] != V[(opcode & 0x00F0)>>4])
            {
                pc+=4;
            }
            else
            {
                pc+=2;
            }
        break;
        case 0xA000:
            I=opcode & 0x0FFF;
            pc+=2;
        break;
        case 0xB000:
            pc=V[0x0]+(opcode & 0x0FFF);
        break;
        case 0xC000:
            V[(opcode & 0x0F00)>>8]=(rand()%255 +1) & ( opcode & 0x00FF);
            pc+=2;
        break;
        case 0xD00:
        //DO LATERRR
        break;
        case 0xE000:
            switch (opcode & 0x000F)
            {
                case 0x000E:
                    unsigned char key_pressed=key[V[(opcode & 0x0F00)>>8]];
                    if(key_pressed)
                        pc+=4;
                    else
                        pc+=2;
                    break;
                case 0x0001:
                    unsigned char key_pressed=key[V[(opcode & 0x0F00)>>8]];
                    if(!key_pressed)
                        pc+=4;
                    else
                        pc+=2;
                break;
                default:
                    printf("Unreognized E OPCODE");
                break;
            }
        break;
        case 0xF000:
            switch (opcode & 0x00FF)
            {
                case 0x0007:
                    V[(opcode & 0x0F00)>>8]=delay_timer;
                break;
                case 0x000A:
                    
                break;
                case 0x0015:
                    delay_timer =  V[(opcode & 0x0F00)>>8];
                    pc+=2;
                break;
                case 0x0018:
                    sound_timer = V[(opcode & 0x0F00)>>8];
                    pc+=2;
                break;
                case 0x001E:
                    I+= V[(opcode & 0x0F00)>>8];
                    pc+=2;
                break;
                case 0x0029:
                    char digit = V[(opcode & 0xF00)>>8];
                    switch (digit & 0x000F)
                    {
                    case 0x0000:
                        I=0;
                        break;
                    case 0x0001:
                        I=5;
                    break;
                    case 0x0002:
                        I=10;
                    break;
                    case 0x0003:
                        I=15;
                    break;
                    case 0x0004:
                        I=20;
                    break;
                    case 0x0005:
                        I=25;
                    break;
                    case 0x0006:
                        I=30;
                    break;
                    case 0x0007:
                        I=35;
                    break;
                    case 0x0008:
                        I=40;
                    break;
                    case 0x0009:
                        I=45;
                    break;
                    case 0x000A:
                        I=50;
                    break;
                    case 0x000B:
                        I=55;
                    break;
                    case 0x000C:
                        I=60;
                    break;
                    case 0x000D:
                        I=65;
                    break;
                    case 0x000E:
                        I=70;
                    break;
                    case 0x000F:
                        I=75;
                    break;
                    default:
                        printf("somethin wrong in opcode FX29");
                        break;
                    }
                    pc+=2;//I=V[(opcode & 0x0F00)>>8]
                break;
            case 0x0033:
                unsigned char change=V[(opcode & 0x0F00)>>8];
                memory[I]=change/100;
                memory[I+1]=(change/10)%10;
                memory[I+2]=(change%100)%10;
                pc+=2;
            break;
            case 0x055:
                short loop_until=V[(opcode & 0x0F00)>>8];
                unsigned short offseet=I;
                for(int i=0;i<=loop_until;i++)
                {
                    memory[offseet]= V[i];
                    offseet++;
                }
            break;
            case 0x0065:
                short loop_until=V[(opcode & 0x0F00)>>8];
                unsigned short offset=I;
                for(int i=0;i<=loop_until;i++)
                {
                    V[i]=memory[offseet];
                    offseet++;
                }
            break;
            default:
                printf("Unrecognized F opcede");
                break;
            }
        break;
        default:
            printf("Unrecognized Opcode\n");
        break;
 
    if(delay_timer>0)
    {
     delay_timer--;
    }
    if(sound_timer>0)
    {
        if(sound_timer==1)
        {
            printf("BRRP");
            sound_timer--;
        }
    }  

};