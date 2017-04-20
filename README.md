# eCli
CLI support for embedded project

## Used on
- tiny mcu like **arm m0** and more advanced system like **stm32f4**

## Why?
- debugging
- test automation
- communication interface
- field assistance
- firmware upgrade

## Example

For the example we will use the default setting (.h)
wich use password login.

### Init
    callbacks:
    uint8_t Cliwrite(uint8_t str){
          //write function here
          return 0;
    }

    uint8_t Cliexitfun(){
      //exit code here
      return 0;
    }
    :callbacks

    eCliCtx eCli; //ctx

    uint8_t eCliBuffer[32]; //rx buffer

    uint8_t mypass[10]; //password

    uint32_t LogOutTime = set your time for auto logout here;

    eCliInit(&**eCli**,"name","nam2","help string",**cliCmdList**,eCliBuffer,sizeof(eCliBuffer),0,&**Cliwrite**,&**Cliexitfun**,mypass,sizeof(mypass),LogOutTime);

###  CLI commands definition (cliCmdList)

**cmd definition (.h)**

    extern const eCliCmdList cliCmdList[];

    uint8_t cli_ls(struct eCliCtx* ctx,int argc,void** argv);
    uint8_t cli_reset(struct eCliCtx* ctx,int argc,void** argv);
    ...

**implementation example(.c) {"cmdname for cli",&cmd,"help"}**

    const eCliCmdList cliCmdList[] = {
      {"ls",&cli_ls,"ls - list file on current directory\nls -l - list file on current directory verbose\nls -v - verbose output md5 sum can be used with -l"},
      {"cd",&cli_cd,"cd PATH - change directory"},
      {"reboot",&cli_reset,"reboot - sync disk and database, then reboot\nreboot disable on error - disable the reboot on error\n reboot enable on error - enable the reboot on error\nreboot force - reboot now warn: database may lost data if writing"},
      {"mount",&cli_mount,"mount - mount the file system"},
      {"tamper",&cli_tamper,"tamper - get tamper value\ntamper init - initialize the tamper"},

      {"mkfs.msdos",&cli_format,"format - inizialize the file system, and reset all memory db. all file are lost"},
      {"stack",&cli_stack,"stack - stack usage"},
      {"su",&cli_su,"nothing for now"},
      {"debug",&cli_debug,"debug - enable debug output\ndebug disable - disable debug"},

      {"read",&cli_read,"read rom|ram VALUESTART VALUESTOP - output hex from ram or rom"},
      {"write",&cli_write,"write rom|ram VALUESTART HEXBUFFER - write the hexbuffer to ram or rom"},
      {"fill",&cli_fill,"fill rom|ram VALUESTART VALUESTOP HEXBUFFER - fill ram or rom with hexbuffer pattern"},

      {"now",&cli_now,""},
      {"date",&cli_date,"date - output the current time unixtime format\ndate forcesync - force act system like the time is correct\ndate add SECONDS - add SECONDS ammount to the current datetime\ndate set SECONDS - set unixtime to the SECONDS amount\
        \ndate stop|start - disable|enable time update\ndate is SECONDS - show human readable unixtime from SECONDS\ndate rtc - output the current time unixtime format from rtc"},

      /*watchdog*/
      {"watchdog",&cli_watchdog,"watchdog - enable wathdog\nwatchdog wait - wait watchdog reset"},
      {"wd",&cli_watchdog,"wd - enable wathdog\nwd wait - wait watchdog reset"},

      /*misc*/
      {"cli",&cli_cli,"cli clear - disable the cli"},
      {"cmd",&cli_cmd,"cmd - disable cli & debug. enable serial command interface"},
      {"pin",&cli_pin,"pin init PXX in|out|an|af - initialize a pin PXX\npin clear PXX - clear a pin PXX and set back to tri-state mode\npin write PXX VALUE- write a value(0,1,analog) to pin PXX"},
      {"uart",&cli_uart,"uart clear 3|6 - clear the uart\nuart init 3|6 VALUESPEED - initialize the uart 3|6 with speed VALUESPEED\nuart write 3|6 HEXBUFFER - write hexbuffer to uart"},
      {"diagnostic",&cli_diagnostic,"diagnostic - output current diagnostic\ndiagnostic make - force send gw diagnostic to noc"},
      {"sdcard",&cli_sd,"sdcard disable cache - disable the sd card IO cache\nsdcard test - test sd card for errors"},

      /*db*/
      {"db",&cli_db,"db mount - mount|create gw database\ndb check DBNUM - check db for errors and attempt repair\ndb load DBNUM FILE - load the db DBNUM with the value from FILE\ndb read DB KEY POSITION - position is optional, key is hex key index on db\ndb insert DB KEY VALUE - insert VALUE into DB using the key KEY"},
      {"cat",&cli_cat,"cat s/string | h/hex FILE - output file, s/h is optional and force output to string or hex. default cat check file first char for string or hex mode"},
      {"rm",&cli_rm,"rm FILE - remove file"},
      {"echo",&cli_echo,"echo FILE \"STRING\" - write a string to file"},

      {0,0}
    };

    uint8_t cli_reset(struct eCliCtx* ctx,int argc,void** argv){
        if (CLI_ARGV(1,"disable on error") || CLI_ARGV(1,"disable error")){
            GW_REBOOT_ON_ERROR = 0;
            return 0;
        }

        if (CLI_ARGV(1,"enable on error") || CLI_ARGV(1,"enable error")){
            GW_REBOOT_ON_ERROR = 1;
            return 0;
        }

        if (CLI_ARGV(1,"force")){
            while (1) { //expect WD
             WatchDogEnable(3); // force enable
             SdCard_WaitLastOp(); //sync disk
             SystemReset(); //reset
            }
            return 0;
        }

        EPRINTF("timer %u",CallBackTimed(reset_timed_callback , 0, Millis(1000)));
        return 0;
    }
    uint8_t cli_ls(struct eCliCtx* ctx,int argc,void** argv){
     FRESULT ret = f_findfirst(&fsdirectory, &fno, ctx->path, "*");
     if (ret == FR_NO_FILE) return 0;
     if (ret != 0) {
       f_closedir(&fsdirectory);
       P_RET(ret);
     }

     uint8_t verbose = 0;
     uint8_t md5 = 0;
     uint8_t md5sum[17];
     md5sum[16] = 0;
     if (CLI_ARGV(1,"-l") || CLI_ARGV(1,"l")){
        verbose = 1;
     }else
     if (CLI_ARGV(2,"-l") || CLI_ARGV(2,"l")){
        verbose = 1;
     }
     if (CLI_ARGV(2,"verbose") || CLI_ARGV(2,"v") || CLI_ARGV(2,"-v")){
        md5 = 1;
     }else
     if (CLI_ARGV(1,"verbose") || CLI_ARGV(1,"v") || CLI_ARGV(1,"-v")){
        md5 = 1;
     }

    while (ret == FR_OK && fno.fname[0]) {         /* Repeat while an item is found */
    #if _USE_LFN
            EPRINTF("%-12s  %s", fno.fname, fno.lfname);
    #else
            EPRINTF("%s", fno.fname);
    #endif

            if (verbose){
                uint8_t out[80];
                EPRINTF(" - %s",ls_format_size(fno.fsize,out));
            }

            if (md5){
                uint8_t namepath[255];
                #if _USE_LFN
                sprintf(namepath,"%s/%-12s %s",ctx->path,fno.fname,fno.lfname);
                #else
                sprintf(namepath,"%s/%s",ctx->path,fno.fname);
                #endif
                uint8_t ret = f_md5path(namepath,md5sum);
                if (ret == 0){
                    EPRINTFBUFF(" - md5: ",md5sum,16);
                }else
                    EPRINTF(" - md5: %s","error open/read");
            }

            EPRINTF("\n");

            ret = f_findnext(&fsdirectory, &fno);               /* Search for next item */
    }
     f_closedir(&fsdirectory);

     return 0;
    }

### Main loop

    //example manage function ->
    uint8_t CliManage(){

        /*manage time cli*/
        if (GetSysTickDMs() > eCliSysTick)
            **eCliManage(&eCli,GetSysTickDMs() - eCliSysTick);**
        eCliSysTick = GetSysTickDMs();

        if (uartByteAvailable(DEBUG_UART_DEF) && SERIAL_CLI_ACTIVE){
            while(uartByteAvailable(DEBUG_UART_DEF))
            **eCliRecvByte(&eCli,uartReceiveByte(DEBUG_UART_DEF));**
        }

        return 0;
    }








