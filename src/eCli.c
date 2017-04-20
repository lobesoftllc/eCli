#include "eCli.h"

#define CLI_MATCH(x) strcmp(stringa,(x)) == 0

static uint8_t eCliWriteNewLine(eCliCtx* ctx);
static uint8_t eCliWriteString(eCliCtx* ctx, uint8_t* str);
static uint8_t eCliWriteCli(eCliCtx* ctx);
static uint8_t eCliProcess(eCliCtx* ctx);
static uint8_t eCliWriteStringLn(eCliCtx* ctx, uint8_t* str);
static uint8_t eCliClearScreen(eCliCtx* ctx);

#ifdef CLI_USE_PASSWORD
static uint8_t eCliLogged(eCliCtx* ctx);
static uint8_t eCliDeLog(eCliCtx* ctx);
static uint8_t eCliLog(eCliCtx* ctx);
#endif


static uint8_t eCliDeLog(eCliCtx* ctx){
  #ifdef CLI_USE_PASSWORD
  if (ctx->lock){
     return 0;
  }
  if (eCliLogged(ctx)){
                //ctx->lock_time = 0;
                ctx->logged = 0;
                ctx->password_err = 0;
                ctx->time_err_password = 0;
                ctx->lock = 0;
                eCliWriteStringLn(ctx,CLI_DISCONNECT_MESSAGE);
     return 0;
  }
  return 0;
#else
  return 0;
#endif
}

static uint8_t eCliLog(eCliCtx* ctx){
  #ifdef CLI_USE_PASSWORD
  if (ctx->lock){
     return 0;
  }
  if (eCliLogged(ctx)==0){
                //ctx->lock_time = 0;
                ctx->logged = 1;
                ctx->password_err = 0;
                ctx->time_err_password = 0;
                ctx->lock = 0;
                eCliWriteStringLn(ctx,CLI_LOGGED_MESSAGE);
                eCliClearScreen(ctx);
     return 0;
  }
  return 0;
#else
  return 0;
#endif
}

uint8_t eCliWriteHelp(eCliCtx* ctx){
	return eCliWriteStringLn(ctx,ctx->CliHelp);
}

static uint8_t* TokenListEcli[]={" ",0};

static uint8_t EasterEgg(eCliCtx* ctx,uint8_t* stringa){
	
	/*@TODO delete me*/
	if (CLI_MATCH(":)")){
		return eCliWriteStringLn(ctx,":D");
	}
	if (CLI_MATCH(":D")){
		return eCliWriteStringLn(ctx,":]");
	}
	if (CLI_MATCH(":*")){
		return eCliWriteStringLn(ctx,"O.O");
	}
	if (CLI_MATCH(":-)")){
		return eCliWriteStringLn(ctx,":-D");
	}
	if (CLI_MATCH(":-D")){
		return eCliWriteStringLn(ctx,":-]");
	}
	if (CLI_MATCH(":-*")){
		return eCliWriteStringLn(ctx,"O.O");
	}

	
	return 1;
}

uint8_t eCliManage(eCliCtx* ctx,uint32_t timepassed){
#ifdef CLI_USE_PASSWORD
  if (ctx->lock){
     return 0;
  }
  
  if (eCliLogged(ctx)){
     ctx->time_err_password+=timepassed;
     
     if (ctx->time_err_password > ctx->lock_time){
        /*lock down*/
        eCliDeLog(ctx);
     }
     
     return 0;
  }
  
  if (ctx->time_err_password >0 && ctx->password_err >0){
      ctx->time_err_password-=timepassed;
      
      if (ctx->time_err_password <=0){
          ctx->time_err_password = 0;
          /*print password error*/
          eCliWriteStringLn(ctx,CLI_ERROR_MESSAGE);
      }
  }
#endif
  
  return 0;
}

static uint8_t eCliProcess(eCliCtx* ctx){
	eCliCmdList* CmdS = ctx->CmdList;
	uint8_t* args[CLI_MAX_ARGS];
	uint16_t args_num = 1;
        uint8_t printcmd = 0;
        uint8_t printcmdhelp = 0;
        
	/*tokenize the input string*/
	uint8_t* netxtoken = ctx->InputBuffer;
	uint8_t* stringa;
        /*skip spaces*/
        stringa = StrStrTok(netxtoken, TokenListEcli ,&netxtoken);
	args[0] = stringa;
	
	if (stringa == 0) {
		stringa = ctx->InputBuffer;
	}
	else{
		/*get args*/
          if (netxtoken){
          while ((args[args_num] = StrStrTok(netxtoken, TokenListEcli ,&netxtoken)) != 0 && args_num < CLI_MAX_ARGS) {
                args_num++;
          }
		args[args_num] = netxtoken;
		args_num++;
          }
	}
	
	args[args_num] = 0;
        
#ifdef CLI_USE_PASSWORD
        if (eCliLogged(ctx)==0){
            /*check password*/
            if (stringa[0] == 0) return 0;
          
            if (strlen(stringa) == ctx->password_size && memcmp(stringa,ctx->password,strlen(stringa)) == 0){
                /*logged*/
                eCliLog(ctx);
                return 0;
            }
            
            ctx->password_err++;
            ctx->time_err_password = CLI_LOCK_TIME;
            if (ctx->password_err > CLI_LOCK_ON_ERROR_RETRY && CLI_LOCK_ON_ERROR_RETRY != 0){
                ctx->lock = 1;
            }
            
            return eCliWriteStringLn(ctx,CLI_WAIT_MESSAGE);
        }else
          ctx->time_err_password = 0; //reset disconnect timer
#endif
        
	/*cose carine*/
	if (!EasterEgg(ctx,stringa)) return 0;
	
	/*check for ?*/
	if (CLI_MATCH("?")) return eCliWriteHelp(ctx);
	else
	if (CLI_MATCH("clear")) return eCliClearScreen(ctx);
	else
          if (CLI_MATCH("compgen") || CLI_MATCH("list")) {
            /*print all cmd list*/
             printcmd = 1;
          }
        else
          if (CLI_MATCH("man")) {
            /*print all cmd list*/
             printcmdhelp = 1;
             if (args_num < 2) return 1;
             stringa = args[1];
        }
	else
	if (CLI_MATCH("exit")){
		uint8_t ret = ctx->exitfun();
		if (ret == 0){
			eCliClearScreen(ctx);
			eCliWriteString(ctx,CLI_EXIT_MESSAGE);
                        eCliDeLog(ctx);
		}
	}else
	if (CLI_MATCH("err") || CLI_MATCH("lasterr") || CLI_MATCH("return code")){
		EPRINTFEx(0,"%u\n",ctx->lastexitcode);
                return 0;
	}
	
	while(CmdS){
                if (CmdS->CmdString == 0) break; 
		if (CLI_MATCH(CmdS->CmdString)){
                    if (printcmdhelp){
                        eCliWriteStringLn(ctx,CmdS->manual);
                        return 0;
                    }
			uint8_t ret = CmdS->callback(ctx,args_num,args);
                        ctx->lastexitcode = ret;
                        //eCliWriteStringLn(ctx,);
                        eCliWriteNewLine(ctx);
			return 0;
		}
                if (printcmd){
                    eCliWriteStringLn(ctx,CmdS->CmdString);
                }
		CmdS++;
	}
        if (!printcmd)
	eCliWriteStringLn(ctx,CLI_ERR_NOT_FOUND);
	return 1;
};

static uint8_t eCliWriteNewLine(eCliCtx* ctx){
	return eCliWriteString(ctx,"\n\r");
};

static uint8_t eCliWriteString(eCliCtx* ctx, uint8_t* str){
	while(str[0]){
		ctx->write(str[0]);
                if (str[0] == '\n')
                    ctx->write('\r');
		str++;
	}
	return 0;
}

static uint8_t eCliClearScreen(eCliCtx* ctx){
	return eCliWriteString(ctx,CLI_CLEAR_SCREEN);
}

static uint8_t eCliWriteStringLn(eCliCtx* ctx, uint8_t* str){
	eCliWriteString(ctx,str);
	eCliWriteNewLine(ctx);
}

static uint8_t eCliLogged(eCliCtx* ctx){
  #ifdef CLI_USE_PASSWORD
  if (ctx->logged && ctx->password_size > 0) return 1;
  if (ctx->password_size == 0) return 1;
  return 0;
  #else
  return 0;
  #endif
}

static uint8_t eCliWriteCli(eCliCtx* ctx){
#ifdef CLI_USE_PASSWORD
  if (eCliLogged(ctx)==0 && ctx->lock==0){
     eCliWriteString(ctx,"password:");
     return 0;
  }
#endif
  
	eCliWriteString(ctx,ctx->CliString);
	eCliWriteString(ctx,"-");
	eCliWriteString(ctx,ctx->CliSysString);
        if (ctx->path != 0){
          eCliWriteString(ctx,"-");
          eCliWriteString(ctx,ctx->path);
        }
	ctx->write(ctx->chlast);
        ctx->write(' ');
        /*write input buffer*/
        eCliWriteString(ctx,ctx->InputBuffer);
        return 0;
}

uint8_t eCliInit(eCliCtx* ctx,uint8_t* CliString,uint8_t* CliSysString,uint8_t* CliHelp,eCliCmdList* CmdList,uint8_t* inputbuffer,
uint32_t inputbuffermax,uint8_t disableout,uint8_t (*write)(uint8_t str),uint8_t (*exitfun)()
#ifdef CLI_USE_PASSWORD
,uint8_t* password,uint8_t password_size,uint32_t lock_time
#endif 
){
	memset(ctx,0,sizeof(eCliCtx));
        
        strcpy(ctx->path,"\\");
        
        #ifdef CLI_USE_PASSWORD
                 ctx->password = password;
                 ctx->password_size = password_size;
                 ctx->lock_time = lock_time;
        #endif
        
	ctx->CliString = CliString;
	ctx->CliSysString = CliSysString;
	ctx->CliHelp = CliHelp;
	ctx->CmdList = CmdList;
	ctx->write = write;
	ctx->exitfun = exitfun;
        ctx->chlast = '>';
	ctx->InputBuffer = inputbuffer;
	ctx->InputBufferMaxSize = inputbuffermax;
	ctx->disableoutdigit = disableout;
	eCliClearScreen(ctx); //clear all
        ctx->lastexitcode = 0;
	return eCliWriteCli(ctx);
}

uint8_t eprint(eCliCtx* ctx,uint8_t* program,uint8_t* str){
    eCliWriteString(ctx,str);
    //eCliWriteNewLine(ctx);
    return 0;
}

uint8_t eprintch(eCliCtx* ctx,uint8_t* program,uint8_t ch){
  if (ch == '\n'){
        ctx->write('\r');
        return 0;
  }
    ctx->write(ch);
    //eCliWriteNewLine(ctx);
    return 0;
}

uint8_t eCliScanSubCmd(eCliCtx* ctx,uint8_t* substr,uint32_t size){
	eCliCmdList* CmdS = ctx->CmdList;
        eCliWriteNewLine(ctx);
        uint8_t* cmdstring = 0;
        uint32_t cmdnum = 0;
        
        /*go till the last space*/
        while(size>1 && substr[size-1] != ' ') size--;
        if (size == 1 && substr[size-1] != ' ') size = 0;
        
	while(CmdS){
		if (CmdS->CmdString == 0) break; 
		
		if (strstr(CmdS->CmdString,substr + size)){
                        if (cmdnum != 0)
                            eCliWriteStringLn(ctx,CmdS->CmdString);
                        if (cmdnum == 0)
                            cmdstring = CmdS->CmdString;
                        cmdnum++;
		}
		CmdS++;
	}
        
        if (cmdnum == 1){
            uint32_t lsize = strlen(cmdstring);
            memmove(ctx->InputBuffer + size,cmdstring,lsize);
            ctx->InputBufferSize = lsize+1 + size;
            ctx->InputBuffer[lsize + size] = ' ';
            ctx->InputBuffer[lsize+1 + size] = 0;
        }else
          eCliWriteStringLn(ctx,cmdstring);
        
        /*reprint*/
        eCliWriteCli(ctx);
	return 0;
}

uint8_t eCliRecvByte(eCliCtx* ctx,uint8_t ch){
	uint8_t ret;
        
#ifdef CLI_USE_PASSWORD
  if (ctx->lock || (ctx->time_err_password && eCliLogged(ctx)==0)){
     return 0;
  }
#endif
	switch(ch){
		/*case CLI_CAR_RETURN:
			//return 0;
			break;*/
		
		case CLI_TAB:
                    #ifdef CLI_USE_PASSWORD
                          if (eCliLogged(ctx)==0){
                          return 0;
                          }
                    #endif
                  
			/*sub scan cmd list*/
			return eCliScanSubCmd(ctx,ctx->InputBuffer,ctx->InputBufferSize);
			break;
		
		case CLI_CAR_RETURN:
		case CLI_NEWLINE:
				ctx->InputBuffer[ctx->InputBufferSize] = 0;
                                if (ch == CLI_CAR_RETURN)
                                    eCliWriteNewLine(ctx);
				ret = eCliProcess(ctx);
				ctx->InputBufferSize = 0;
				ctx->InputBuffer[ctx->InputBufferSize] = 0;
				ret = eCliWriteCli(ctx);
				return ret;
			break;
		case CLI_BACKSPACE:
			if (ctx->InputBufferSize > 0){
				ctx->InputBufferSize--;
			}else
                            return 0;
			break;
		default:
			if (ctx->InputBufferSize < ctx->InputBufferMaxSize){
				ctx->InputBuffer[ctx->InputBufferSize] = ch;
				ctx->InputBufferSize++;
                                ctx->InputBuffer[ctx->InputBufferSize] = 0;
			}else return 1;
	}
  if (!ctx->disableoutdigit){
                    #ifdef CLI_USE_PASSWORD
                          if (eCliLogged(ctx)==0){
                          return ctx->write('*');
                          }
                    #endif
                          return ctx->write(ch);
  }
	return 0;
}

int8_t eClistrcmp(void* str1,void* str2){
  if (str1 == 0) return -1;
  if (str2 == 0) return 1;
  return strcmp(str1,str2);
}