#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

/* Driver Header files */
#include <ti/display/Display.h>
#include <ti/drivers/NVS.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#define FOOTER "=================================================="
#define MAXBUFLEN 50

char buffer[MAXBUFLEN];
char rbuffer[MAXBUFLEN];
char buf_fc[MAXBUFLEN]; //for file counter loop

int i = 0;

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    uint_fast16_t wstatus, rstatus;
    NVS_Handle nvsHandle;
    NVS_Attrs regionAttrs;
    NVS_Params nvsParams;
    Display_Handle displayHandle;

    Display_init();
    NVS_init();
    displayHandle = Display_open(Display_Type_UART, NULL);
    if (displayHandle == NULL)
    {
        /* Display_open() failed */
        while (1);
    }

    NVS_Params_init(&nvsParams);
    nvsHandle = NVS_open(CONFIG_NVSEXTERNAL, &nvsParams);
    if (nvsHandle == NULL)
    {
        Display_printf(displayHandle, 0, 0, "NVS_open() failed.");
        return (NULL);
    }

    uint16_t newLen = 0;
    uint16_t totalRead = 0;
    uint16_t readRound = 0;
    uint16_t round = 0;
    uint16_t roundn = 0; //for final nvs read data display @line 117
    int fileSize = 0;
    uint8_t filecount = 11;
    uint8_t fc = 1; //for file counter loop

    int store[filecount][2]; //for store file sizes
    int arrcnt = 0; //array counter

    NVS_getAttrs(nvsHandle, &regionAttrs);
    size_t round_offset = 4;

    Display_printf(displayHandle, 0, 0, "\n ====================MAIN START==================== \n");

    //erase everything at first
    NVS_erase(nvsHandle, 0, 0x100000);
    NVS_write(nvsHandle, 0, 0, sizeof(MAXBUFLEN+1), NVS_WRITE_ERASE | NVS_WRITE_POST_VERIFY);
    memset(rbuffer, 0, (MAXBUFLEN));
    memset(buffer, 0, (MAXBUFLEN));
    memset(buf_fc, 0, (MAXBUFLEN));

    for(fc=1; fc <= filecount;)
    {
          char location[] = "/home/pramodh/file_read1/rdfile";
          snprintf(buf_fc, sizeof(buf_fc), "%d", fc);
          snprintf(buf_fc, sizeof(buf_fc), "%s%d.txt", location,fc);
          // Display_printf(displayHandle, 0, 0, "%s", buf_fc);

          // FILE *fp = fopen("/home/pramodh/file_read/rdfile2.txt", "r");
          FILE *fp = fopen(buf_fc, "r");

    if (fp != NULL)
    {
        fseek(fp, 0, SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0L, SEEK_SET);
        Display_printf(displayHandle, 0, 0, "\n========================================");
        Display_printf(displayHandle, 0, 0, "Accessing File%d ", fc);
        Display_printf(displayHandle, 0, 0, "Total file size = %d ", fileSize);

        readRound = ((fileSize % MAXBUFLEN) == 0)? 0 :1 ;
        readRound += (fileSize/MAXBUFLEN);
        Display_printf(displayHandle, 0, 0, "Need to read %d times \n", readRound);

        store[arrcnt][0] = fileSize;
        store[arrcnt][1] = round_offset;
        arrcnt = arrcnt + 1;

    while (round < (readRound+1))
        {
        memset(rbuffer, 0, (MAXBUFLEN));
        memset(buffer, 0, (MAXBUFLEN));
        memset(buf_fc, 0, (MAXBUFLEN));
        //Display_printf(displayHandle, 0, 0, "Writing round %d-%d", fc,round);
        //Read from file and store on buffer here
        fseek(fp, 0 + round * MAXBUFLEN, SEEK_CUR);
        newLen = fread(buffer, sizeof(char), MAXBUFLEN, fp);

        //Write to NVS
        wstatus = NVS_write(nvsHandle, round_offset, (void *) buffer, sizeof(buffer), NVS_WRITE_POST_VERIFY);
//        rstatus = NVS_read(nvsHandle, round_offset, rbuffer, sizeof(buffer));
//        Display_printf(displayHandle, 0, 0, "%s \n", rbuffer);

        totalRead += newLen;

        memset(rbuffer, 0, (MAXBUFLEN));
        memset(buffer, 0, (MAXBUFLEN));
        round_offset = (MAXBUFLEN + 1)+ round_offset;
        round++;
        fseek(fp, 0L, SEEK_SET);
        }

    if (ferror(fp) != 0)
       {
           Display_printf(displayHandle, 0, 0, "Error reading file");
       }
       else
       {
           buffer[newLen++] = '\0';
       }
       readRound += MAXBUFLEN;
    }
    Display_printf(displayHandle, 0, 0, "File%d Completed", fc);
       fclose(fp);
       fc= fc + 1;
       round = 0;
       totalRead = 0;
       newLen = 0;
       memset(rbuffer, 0, (MAXBUFLEN));
       memset(buffer, 0, (MAXBUFLEN));
       memset(buf_fc, 0, (MAXBUFLEN));
       sleep(1);

    } // end of writing for-loop

    sleep(1);
    fc= fc - 1;

    //display all chunks at once after writing
    if(fc == filecount)
    Display_printf(displayHandle, 0, 0, "\n Writing completed %d files written \n", fc);

    {
       arrcnt = 0;
       for(arrcnt = 0; arrcnt < filecount;)
       {
            Display_printf(displayHandle, 0, 0, "\n========================================");
            Display_printf(displayHandle, 0, 0, "Reading File%d with %d bytes >>> Start at offset 0x%x", (arrcnt + 1), store[arrcnt][0], store[arrcnt][1]);

            uint16_t displayRound=0;
            displayRound = (((store[arrcnt][0]) % MAXBUFLEN) == 0)? 0 :1 ;
            displayRound += ((store[arrcnt][0])/MAXBUFLEN);
            size_t round_offset = store[arrcnt][1];

    while (roundn < (displayRound))
    {
            Display_printf(displayHandle, 0, 0, "Reading round %d/%d \tfrom address: 0x%x ", roundn, displayRound, round_offset);
            rstatus = NVS_read(nvsHandle, round_offset, rbuffer, sizeof(buffer));
            Display_printf(displayHandle, 0, 0, "%s \n", rbuffer);
            round_offset = (MAXBUFLEN + 1) + round_offset;
            memset(rbuffer, 0, (MAXBUFLEN));
            roundn++;
    }

            arrcnt = arrcnt + 1;
            round_offset = store[arrcnt][1];
            roundn = 0;
            sleep(1);
       } //end of reading for loop

           Display_printf(displayHandle, 0, 0, "END of elements Reading");
           sleep(1);

    }

        //erase everything at end
        NVS_erase(nvsHandle, 0, 0x100000);
        NVS_write(nvsHandle, 0, 0, sizeof(MAXBUFLEN+1), NVS_WRITE_ERASE | NVS_WRITE_POST_VERIFY);
        memset(rbuffer, 0, (MAXBUFLEN));
        memset(buffer, 0, (MAXBUFLEN));
        sleep(1);
        Display_printf(displayHandle, 0, 0, "=============All Erased===============");

      NVS_close(nvsHandle);
      return (NULL);
}
