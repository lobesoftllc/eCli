#ifndef ECLI_COMMON_H_
#define ECLI_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdint.h>
#include "../common/com_str.h"

#define CLI_TAB 0x09
#define CLI_BACKSPACE 0x7F
#define CLI_NEWLINE '\n'
#define CLI_CAR_RETURN 0x0D
#define CLI_CLEAR_SCREEN "\033[2J\033[;H"

#define CLI_ERR_NOT_FOUND "ERR: command not found"
#define CLI_EXIT_MESSAGE "By By"
#define CLI_ERROR_MESSAGE "error"
#define CLI_DISCONNECT_MESSAGE "disconnected"
#define CLI_LOGGED_MESSAGE "logged"
#define CLI_WAIT_MESSAGE "wait"


#define CLI_MAX_ARGS 12
  
#define CLI_USE_PASSWORD

#define CLI_LOCK_ON_ERROR_RETRY 3
#define CLI_LOCK_WAIT_ON 3
#define CLI_LOCK_TIME (6000*10)

typedef struct eCliCtx{
        /*password login*/
#ifdef CLI_USE_PASSWORD
        uint8_t logged;
        uint8_t password_err;
        uint32_t time_passed;
        uint32_t lock_time;
        int32_t time_err_password;
        uint8_t* password;
        uint8_t password_size;
        uint8_t lock;
#endif
        
	uint8_t* CliString;
	uint8_t* CliSysString;
	uint8_t* CliHelp;
        
        uint8_t path[255];
        
        uint8_t chlast;
	
	uint8_t disableoutdigit;
	
        uint8_t printfbuffer[255];
        
	uint8_t* InputBuffer;
	uint32_t InputBufferSize;
	uint32_t InputBufferMaxSize;
	
	struct eCliCmdList* CmdList; //null terminated list
	
	uint8_t (*write)(uint8_t str);
	uint8_t (*exitfun)();
        
        uint8_t lastexitcode;
}eCliCtx;

typedef struct eCliCmdList{
	uint8_t* CmdString;
	uint8_t (*callback)(struct eCliCtx* ctx,int argc,void** argv);
        uint8_t* manual;
}eCliCmdList;

uint8_t eCliInit(eCliCtx* ctx,uint8_t* CliString,uint8_t* CliSysString,uint8_t* CliHelp,eCliCmdList* CmdList,uint8_t* inputbuffer,
                 uint32_t inputbuffermax,uint8_t disableout,uint8_t (*write)(uint8_t str),uint8_t (*exitfun)()
                  #ifdef CLI_USE_PASSWORD 
                   ,uint8_t* password,uint8_t password_size,uint32_t lock_time 
                  #endif 
                  );
                     
uint8_t eCliRecvByte(eCliCtx* ctx,uint8_t ch);

uint8_t eCliManage(eCliCtx* ctx,uint32_t timepassed);

uint8_t eprint(eCliCtx* ctx,uint8_t* program,uint8_t* str);
uint8_t eprintch(eCliCtx* ctx,uint8_t* program,uint8_t ch);

int8_t eClistrcmp(void* str1,void* str2);

#define EPRINTF(x...){\
    sprintf(ctx->printfbuffer,x);\
    eprint(ctx,argv[0],ctx->printfbuffer);\
}

#define EPRINTSTR(x){\
    eprint(ctx,argv[0],x);\
}

#define EPRINTCH(x){\
    eprintch(ctx,argv[0],x);\
}

#define EPRINTFEx(y,x...){\
    sprintf(ctx->printfbuffer,x);\
    eprint(ctx,y,ctx->printfbuffer);\
}

#define EPRINTFBUFF(xn, xb, xl) do { \
		if (!(xb)) {EPRINTF("%s is NULL\n\r", (xn)); break; } \
                EPRINTF("%s: ",xn);\
                int i;\
                for(i=0;i<xl;i++) EPRINTF("%02x",xb[i]);\
                EPRINTF("\n");\
                } while (0)

#define CLI_ARGV(x,value) eClistrcmp(argv[x],value) == 0
#define CLI_ARGV_NCHECK(x){\
if (x+1 > argc) {\
  EPRINTF("wrong arguments");\
  return 1;\
}\
}

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* ECLI_COMMON_H_ */