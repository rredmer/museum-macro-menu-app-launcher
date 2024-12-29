/*---------------------------------------------------------------------------
**
**  TITLE:  "MACRO-MENU"  
**
**  (c) 1988 MANAGEMENT SYSTEMS SOFTWARE, INC.
**  BY: RONALD D. REDMER
**  VICE PRESIDENT, RESEARCH AND DEVELOPMENT
**
**  VERSION 1.0,  RELEASE DATE: 12/7/88
**
**  SOURCE:	
**    MENUPROG.C  - UTILITY TO START APPLICATIONS VIA MENU.
**
**  LIBRARIES:
**    WFD & WFC - (c) VERMONT CREATIVE SOFTWARE.
**  
**  GUIDE TO COMMENTS:
**    (PRE)  - PREPROCCESSOR
**    (STD)  - INCLUDED IN STANDARD MICROSOFT C.
**    (INT)  - LOCAL TO INTERNAL SOURCE CODE (i.e. THIS MODULE)
**    (WFD)  - INCLUDED IN "WINDOWS FOR DATA" BY VCS.
**    (WFC)  - INCLUDED IN "WINDOWS FOR C" BY VCS.
**
**-------------------------------------------------------------------------*/
#define LINT_ARGS															/* STD ENABLE ARGUMENT TYPE CHECKING - COMPILER FLAG */		 
#include <stdio.h>														/* STD REQUIRED FOR DISK IO */
#include <string.h>														/* STD REQUIRED FOR STRING FUNCTIONS */
#include <stdlib.h>														/* STD REQUIRED FOR ERROR TRAPPING */
#include <process.h>														/* STD REQUIRED FOR SUB-PROCESS CONTROL */
#include <wfd.h>															/* WFD SYSTEM INCLUDE FILE */
#undef F_DATE																/* WFD UNDEF. TO SAVE SPACE */
#undef F_TIME																/* WFD UNDEF. TO SAVE SPACE */
#undef F_CHAR																/* WFD UNDEF. TO SAVE SPACE */
#undef F_BOOL																/* WFD UNDEF. TO SAVE SPACE */
#undef F_LONG																/* WFD UNDEF. TO SAVE SPACE */
#undef F_MENU																/* WFD UNDEF. TO SAVE SPACE */
#undef F_DECIMAL															/* WFD UNDEF. TO SAVE SPACE */
#include <wfd_glob.h>													/* WFD GLOBAL VARIABLES */
#include "menuprog.h" 													/* INT LOCAL FUNCTION PROTOTYPES - CREATED W. Zg COMPILER OPTION */

int diskstat(int);														/* ESS DOS CRITICAL ERROR HANDLER DIS/ENABLE FUNCTION PROTOTYPE */

#define MAXMENUITEM  50													/* DETERMINES THE MAXIMUM NUMBER OF MENU ITEMS */
#define CLEAR_ITEM    1													/* INT KEY FUNCTION FLAG FOR CLEARING DATA ITEM */
#define LAUNCH_ITEM   2													/* INT KEY FUNCTION FLAG FOR LAUNCHING INTO APPLICATION */
#define ENTER_PARMS   3													/* INT KEY FUNCTION FLAG FOR ENTERING PROGRAM PARMS */
#define HELP_FILE     "MENU.HLP"										/* INT NAME OF HELP FILE */
#define SETUP_FILE    "MENU.TXT"										/* INT NAME OF SETUP INFORMATION FILE */
#define MEMOFILENAME  "MENUTEMP.BAT"								/* INT NAME OF MEMO FILE FOR CALLING PROGRAM */
#ifndef DEMO																/* INT IF DEMO FLAG NOT SPECIFIED ON COMPILE LINE */
#define PROGRAM_TITLE " MACRO-MENU "								/* SET THE PROGRAM TITLE FOR NON-DEMO COPY */
#endif																		/* END IF */	
#ifdef DEMO																	/* IF THE DEMO FLAG WAS SPECIFIED ON THE COMPILE LINE */	
#define PROGRAM_TITLE    " MACRO-MENU DEMONSTRATION "			/* SET THE PROGRAM TITLE FOR DEMO COPY */
#endif																		/* END IF */
WINDOW MainWindow;														/* WFD POINTER TO SETUP WINDOW (FULL SCREEN WINDOW) */
DFORMPTR LaunchForm;														/* WFD POINTER TO SETUP INFO SCROLLABLE SUBFORM */
char *ProcessName[MAXMENUITEM];										/* PROGRAM NAMES AS THEY APPEAR IN THE LAUNCH MENU */
char *DirPath[MAXMENUITEM];											/* PROGRAM DIRECTORY SEARCH PATHS FOR EACH MENU APPLICATION */
char *ExecName[MAXMENUITEM];											/* EXECUTABLE NAME FOR EACH PROGRAM */
char *ExecComment[MAXMENUITEM];										/* COMMENT FOR EXECUTABLE PROGRAM */
char DefaultPath[101];													/* DEFAULT DISK DIRECTORY PATH */
char DefaultDrive[3];													/* DEFAULT DISK DRIVE SPECIFICATION */
char UserMessage1[25];													/* USER DEFINED SCREEN SAVER MESSAGE - TOP ROW */
char UserMessage2[25];													/* USER DEFINED SCREEN SAVER MESSAGE - BOTTOM ROW */
char CurrentDate[11];													/* HOLDS THE CURRENT SYSTEM DATE */
int Index[MAXMENUITEM];													/* POINTER TO INTEGER ARRAY */
int FuncFlag=0;															/* FLAG INDICATING SPECIFIC KEY FUNCTION */
int ExitFlag=0;															/* FLAG INDICATING PROGRAM EXIT (ESC KEY PROCESSED) */
int CurRow=0;																/* VALUE OF CURRENT ROW IN SCROLLABLE SUBFORMS */
int ProcNum=0;																/* NUMBER OF PROCEDURE TO LAUNCH IN ARRAY */
int ScreenSaveFlag=OFF;          									/* FLAG DETERMINES IF SCREEN SAVE MESSAGE IS ON SCREEN */
int HelpFlag=OFF;
long LastKeyTime=0L;	 													/* TIME TO INDICATE SCREEN SAVE OPERATION NECCESSARY */
/*---------------------------------------------------------------------------
**
**   MAIN
**
**
*/
int main()
{
int ret=0;
datt_tbl[LHELP][1] = c_att(BLACK, WHITE);								/* SET CUSTOM ATTRIBUTE FOR WFD LOGICAL ATTRIBUTE TABLE */
init_wfd();																		/* WFD INITIALIZE SYSTEM **** MUST BE FIRST FUNCTION CALL */
getcwd(DefaultPath, 80);													/* STD GET CURRENT WORKING DIRECTORY PATH */
sprintf(DefaultDrive, "%c:", DefaultPath[0]);						/* STD STORE CURRENT DRIVE SPEC */
Setup(ON);																		/* INT CONFIGURE OPERATING ENVIRONMENT */
while (!ExitFlag)																/* INT WHILE GLOBAL EXIT FLAG IS NOT ACTIVE */
	{
	FuncFlag=OFF;																/* CLEAR FUNCTION FLAG AFTER KEY FUNCTION PROCESSING */
	time(&LastKeyTime); 			   										/* STD GET CURRENT SYSTEM TIME */
	keyd_def(-K_PGUP, kdpgup);												/* WFD DEFINE PAGE UP KEY TO PROPER WFD FUNCTION */
	keyd_def(-K_PGDN, kdpgdn);												/* WFD DEFINE PAGE DOWN KEY TO PROPER WFD FUNCTION */
	keyd_def(K_ENTER, Launch);												/* WFD DEFINE ENTER KEY TO DEFINE PARAMETERS FUNCTION */
	keyd_def(K_ESC, QuitProgramKey); 									/* WFD DEFINE ESC KEY TO INTERNAL EXIT ROUTINE */
	keyd_def(-K_F1, MainHelp);												/* WFD DEFINE F1 TO INTERNAL HELP ROUTINE */
	keyd_def(-K_F2, EnterParmsKey);										/* WFD DEFINE F2 TO CLEAR MENU ITEM ROUTINE */
	keyd_def(-K_F3, ClearItemKey);										/* WFD DEFINE F3 TO CLEAR MENU LIST ROUTINE */
	keyd_def(-K_F4, SetMessage);											/* WFD DEFINE F5 TO SET SCREEN SAVER MESSAGES */
	keyd_def(-K_F10, DisplaySerial); 									/* WFD DEFINE F10 TO DISPLAY SERIAL MESSAGE */
	scfm_rd(0, LaunchForm);													/* WFD READ SCROLLABLE DATA FORM */
	keyd_def(-K_PGUP, kdpfld);												/* WFD RESET KEY FUNCTION POINTER TO DEFAULT */
	keyd_def(-K_PGDN, kdnfld);												/* WFD RESET KEY FUNCTION POINTER TO DEFAULT */
	keyd_def(K_ENTER, kdnfld);												/* WFD RESET KEY FUNCTION POINTER TO DEFAULT */
	keyd_def(K_ESC,   kdexit);												/* WFD RESET KEY FUNCTION POINTER TO DEFAULT */
	keyd_def(-K_F1,   NULLFP);												/* WFD REMOVE KEY FUNCTION POINTER */
	keyd_def(-K_F2,   NULLFP);												/* WFD REMOVE KEY FUNCTION POINTER */
	keyd_def(-K_F3,   NULLFP);												/* WFD REMOVE KEY FUNCTION POINTER */
	keyd_def(-K_F4,   NULLFP);												/* WFD REMOVE KEY FUNCTION POINTER */
	keyd_def(-K_F10,  NULLFP);												/* WFD REMOVE KEY FUNCTION POINTER */
	switch(FuncFlag)															/* INT TEST GLOBAL KEY FUNCTION FLAG */
		{
		case CLEAR_ITEM:														/* IF USER SELECTED CLEAR SINGLE ITEM OPTION */
			DeleteOption();													/* DELETE SINGLE ITEM FROM MENU LIST */
			scfm_dn(LaunchForm);												/* WFD PLACE THE SCROLLABLE SUBFORM ON DISPLAY */
			scfm_up(LaunchForm);												/* WFD PLACE THE SCROLLABLE SUBFORM ON DISPLAY */
			break;																/* BREAK SWITCH */
		case ENTER_PARMS:														/* IF USER SELECTED TO ENTER PROGRAM PARMS FOR AN OPTION */
			EnterParms();														/* INT CALL PROGRAM PARM ENTRY ROUTINE */
			scfm_dn(LaunchForm);												/* WFD PLACE THE SCROLLABLE SUBFORM ON DISPLAY */
			scfm_up(LaunchForm);												/* WFD PLACE THE SCROLLABLE SUBFORM ON DISPLAY */
			break;																/* BREAK SWITCH */
		case LAUNCH_ITEM:														/* IF USER SELECTED TO LAUNCH AN APPLICATION */
			{
			if (LaunchProgram()) goto EXIT;
			}
		}
	}
ret = 1;
EXIT:
Setup(OFF);																		/* INT CLEAR ALL LOCAL DATA SPACE */
return(ret);																		/* RETURN TO CALLING PROGRAM W. NO ERRORS */
}
/*---------------------------------------------------------------------------
**
**  SETUP
**
**
*/
int Setup(status)
int status;																			/* FLAG DETERMINING TO CONFIGURE OR DIS-CONFIGURE */
{
DFIELDPTR fp;																		/* WFD POINTER TO DATA ENTRY FIELD */
int cnt;																				/* COUNTER */
if (status)																			/* IF FLAG SPECIFIES CONFIGURATION PROCESS */
	{																					/* THEN SETUP OPERATING ENVIRONMENT */
	se_keyloop(CheckScreen);													/* WFC SET KEYLOOP FUNCTION TO CHECK SCREEN SAVE */
	diskstat(OFF);																	/* ESS DISABLE THE DOS CRITICAL ERROR HANDLER */
	for (cnt=0; cnt<MAXMENUITEM; cnt++)										/* LOOP FOR EACH MENU ITEM */
		{				
		ProcessName[cnt] = stblank(41);										/* WFD ALLOCATE STRING SPACE FOR PROCESS NAMES */
		DirPath[cnt] = stblank(81);											/*	WFD ALLOCATE STRING SPACE FOR SEARCH PATHS */
		ExecName[cnt] = stblank(81);											/* WFD ALLOCATE STRING SPACE FOR EXECUTABLE NAMES */
		ExecComment[cnt] = stblank(81);										/* WFD ALLOCATE STRING SPACE FOR COMMENT */
		Index[cnt] = cnt+1;														/* INITIALIZE INDEX ARRAY */
		}
	defs_wn(&MainWindow, 0, 0, v_rwq, v_coq, BDR_DLNP);
	sw_name(PROGRAM_TITLE, &MainWindow);
	sw_namelocation(TOPCENTER, &MainWindow);
	sw_popup(ON, &MainWindow);
	LaunchForm = scfm_def(2, CENTER_WN, 12, 45, LHELP, BDR_DLNP, MAXMENUITEM);
	sfm_opt(FORMCLEAR, OFF, LaunchForm);
	sw_bdratt(LNORMAL, LaunchForm->wnp);
	fp = scf_def(0, "99", F_INT, (char **) Index, LaunchForm);
	sf_atts(LHELP, LFIELDA, fp);
	sf_opt(SKIP, ON, fp);
	fp = scf_def(3, def_pic('X', 40), F_STRING, ProcessName, LaunchForm);
	sf_atts(LHELP, LFIELDA, fp);
	sf_opt(PROTECTED, ON, fp);
	GetFile();																		/* INT GET THE CURRENT SETUP DATA FILE */
	set_wn(&MainWindow);															/* WFC SET MAIN WINDOW ON THE DISPLAY */
	get_date(CurrentDate);														/* WFC GET CURRENT SYSTEM DATE */
	v_plst(0, 65, CurrentDate, &MainWindow);								/* WFC DISPLAY DATE ON MAIN WINDOW */
	PlaceMessages();																/* INT PLACE HELP MESSAGES INTO WINDOWS */
	scfm_up(LaunchForm);		  													/* WFD PLACE THE SCROLLABLE SUBFORM ON DISPLAY */
	}
else
	{
	scfm_dn(LaunchForm);		      											/* WFD REMOVE THE LAUNCH DATA ENTRY FORM */
	fm_free(LaunchForm);		 													/* WFD DISALLOCATE MEMORY USED FOR FORM */
	unset_wn(&MainWindow);														/* WFC REMOVE MAIN WINDOW */
	se_keyloop(NULLFP);															/* WFC REMOVE KEYLOOP BACKGROUND FUNCTION */
	diskstat(ON);																	/* ESS ENABLE DOS CRITICAL ERROR HANDLER  */
	system("CLS");
	for (cnt=0; cnt<MAXMENUITEM; cnt++)										/* INT LOOP THROUGH EACH MENU ITEM */
		{
		free_mem(ProcessName[cnt]);											/* WFD FREE MEMORY SPACE */
		free_mem(DirPath[cnt]);													/* WFD FREE MEMORY SPACE */
		free_mem(ExecName[cnt]);												/* WFD FREE MEMORY SPACE */
		free_mem(ExecComment[cnt]);											/* WFD FREE MEMORY SPACE */
		}
	}
return(0);
}
/*---------------------------------------------------------------------------
**
**   LAUNCH PROGRAM
**
*/
int LaunchProgram()
{
char buf[200];
char drivespec[100];
char tmpname[200];
int ret;
if (!isblank(ProcessName[ProcNum]))
	{
	ret=access(DirPath[ProcNum], 0);		/* STD ATTEMPT TO ACCESS DIRECTORY */
	if (ret)										/* IF ACCESS ATTEMPT FAILED */
		{											/* CALL FLAG ERROR TO NOTIFY USER */
		FlagError("(Execute) Error finding application directory.");
		return(0);
		}
	sscanf(ExecName[ProcNum], "%8s", buf);
	
	if (isblank( DirPath[ProcNum]+4 ))
		{
		sprintf(drivespec, "%c:", DirPath[ProcNum][0]);
		}
	else
		{
		strcpy(drivespec, DirPath[ProcNum]);
		}

	sprintf(tmpname, "%s\\%s.exe", drivespec, buf); 

	del_imbl(tmpname);
	ret=access(tmpname, 0);
	if (ret)
		{
		sprintf(tmpname, "%s\\%s.com", drivespec, buf);
		del_imbl(tmpname);
		ret=access(tmpname, 0);
		if (ret)
			{
			sprintf(tmpname, "%s\\%s.bat", drivespec, buf);
			del_imbl(tmpname);
			ret=access(tmpname, 0);
			if (ret)
				{
				FlagError("(Execute) Error finding application.");
				return(0);
				}
			}
		}
	UpdateMemoFile(DirPath[ProcNum], ExecName[ProcNum]);
	return(1);												/* RETURN W. SUCCESSFULL LAUNCH CODE */
	}
return(0);
}
/*---------------------------------------------------------------------------
**
**   UPDATE MEMO FILE
**
**
*/
int UpdateMemoFile(buf1, buf2)
char buf1[];
char buf2[];
{
FILE *MemoFile;																/* STD POINTER TO MEMO FILE */
MemoFile=fopen(MEMOFILENAME, "w+");										/* STD OPEN MEMO FILE FOR WRITING */
fprintf(MemoFile, "ECHO OFF\nCLS\n%c:", buf1[0]);
fprintf(MemoFile, "\nCD %s", buf1);
fprintf(MemoFile, "\nCOMMAND /C %s", buf2);
fprintf(MemoFile, "\n%c:", DefaultDrive[0]);
fprintf(MemoFile, "\nCD %s\n", DefaultPath);
fclose(MemoFile);																/* STD CLOSE THE MEMO FILE */
return(0);
}
/*---------------------------------------------------------------------------
**
**  ENTER PARMS KEY
**
**
*/
int EnterParmsKey()
{
FuncFlag=ENTER_PARMS;					/* SET FUNCTION KEY FLAG TO ENTER PROGRAM PARMS */
CurRow = CURSCFMRW;						/* WFC STORE CURRENT SCROLLABLE ROW */
fld_setexit(EXIT_FORM, CURFLDNUM);	/* WFC SET FOR FORM EXIT UPON RETURN */
return(1);
}
/*---------------------------------------------------------------------------
**
**  ENTER PARMS
**
**
*/
int EnterParms()
{
DFORMPTR ParmForm;
DFIELDPTR fp;
char *dir_pic;
dir_pic = def_pic('!', 76);
*(dir_pic+1)=':';
*(dir_pic+2)='\\';
ParmForm = fm_def(v_rwq-16, 0, 16, 80, LDOS, BDR_DLNP);
sw_name(" APPLICATION PARAMETERS ", ParmForm->wnp);
sw_namelocation(TOPCENTER, ParmForm->wnp);
sw_popup(ON, ParmForm->wnp);
sfm_opt(VERIFYEXIT+AUTOEXIT, OFF, ParmForm);
fp = fld_def(1, 1, "APPLICATION DESCRIPTION:", FBELOW, def_pic('X', 40), F_STRING, ProcessName[CurRow], ParmForm);
sf_atts(LREVERSE, LFIELDA, fp);
fp = fld_def(4, 1, "DIRECTORY PATH:", FBELOW, dir_pic, F_STRING, DirPath[CurRow], ParmForm);
sf_atts(LREVERSE, LFIELDA, fp);
fp = fld_def(7, 1, "COMMAND LINE:", FBELOW, def_pic('!',  76), F_STRING, ExecName[CurRow], ParmForm);
sf_atts(LREVERSE, LFIELDA, fp);
fp = fld_def(10, 1, "APPLICATION COMMENT:", FBELOW, def_pic('X',76), F_STRING, ExecComment[CurRow], ParmForm);
sf_atts(LREVERSE, LFIELDA, fp);
ftxt_def(13, CENTER_TEXT, "Press <Esc> to exit.", LDOS, ParmForm);
fm_proc(0, ParmForm);				/* WFD PROCESS APPLICATION PARM DATA ENTRY FORM */
SaveFile();							/* INT SAVE CURRENT DATA FILE */
free_mem(dir_pic);
return(1);					
}
/*---------------------------------------------------------------------------
**
**  PLACE MESSAGES
**
**
*/
int PlaceMessages()
{
v_plst(14, 1, "<F1> Help.", &MainWindow);
v_plst(15, 1, "<F2> Edit menu option.", &MainWindow);
v_plst(16, 1, "<F3> Remove option from menu.", &MainWindow);
v_plst(17, 1, "<F4> Define screen save message.", &MainWindow);
v_plst(19, 1, "<Esc> Exit to DOS.", &MainWindow);
v_plst(20, 1, "<Enter> Execute option.", &MainWindow);

sw_att(LREVERSE, &MainWindow);
v_plst(v_rwq-3, 0, "      (c) 1988  MANAGEMENT SYSTEMS SOFTWARE, INC.  ALL RIGHTS RESERVED.     ", &MainWindow);
sw_att(LNORMAL, &MainWindow);
return(0);
}
/*---------------------------------------------------------------------------
**
**   GET FILE
**
**
*/
int GetFile()
{
FILE *SetupFile;						/* STD POINTER TO SETUP INFORMATION DATA FILE */
int cnt;
SetupFile = fopen(SETUP_FILE, "r+");
if (SetupFile == NULL)
	{
	FlagError("(GetFile) Error Opening File.");
	}
else
	{
	for (cnt=0;cnt<MAXMENUITEM;cnt++)
		{
		fread(ProcessName[cnt], sizeof(char), 40, SetupFile);
		fread(DirPath[cnt],     sizeof(char), 80, SetupFile);
		fread(ExecName[cnt],    sizeof(char), 80, SetupFile);
		fread(ExecComment[cnt], sizeof(char), 80, SetupFile);
		}
	fclose(SetupFile);
	}
return(0);
}
/*---------------------------------------------------------------------------
**
**   SAVE FILE
**
**
*/
int SaveFile()
{
#ifndef DEMO
	FILE *SetupFile;						/* STD POINTER TO SETUP INFORMATION DATA FILE */
	int cnt;
	SetupFile = fopen(SETUP_FILE, "w+");
	if (SetupFile == NULL)
		{
		FlagError("(SaveFile) Error Opening File.");
		}
	else
		{
		for (cnt=0;cnt<MAXMENUITEM;cnt++)
			{
			fwrite(ProcessName[cnt], sizeof(char), 40, SetupFile);
			fwrite(DirPath[cnt],     sizeof(char), 80, SetupFile);
			fwrite(ExecName[cnt],    sizeof(char), 80, SetupFile);
			fwrite(ExecComment[cnt], sizeof(char), 80, SetupFile);
			}
		fclose(SetupFile);
		}
#endif
return(0);
}
/*---------------------------------------------------------------------------
**
**  MAIN HELP
**
*/
int MainHelp()
{
WINDOW hwn;												/* WFD WINDOW STRUCTURE */
MFILEPTR hfl;											/* WFD MEMORY FILE POINTER */
HelpFlag = ON;
defs_wn(&hwn, 1, 0, v_rwq-1, v_coq, BDR_DLNP);
sw_popup(ON, &hwn);
sw_name(" HELP INFORMATION ", &hwn);
sw_namelocation(TOPCENTER, &hwn);
hfl = mf_def(HELP_FILE, 100, 78);		/* WFD DEFINE MEMORY FILE */
mf_rd(hfl);										/* WFD READ MEMORY FILE */
sw_mf(hfl, &hwn);								/* WFD SET MEMORY FILE TO WINDOW */
vs_mf(K_ESC, &hwn);							/* WFD VIEW SCROLLABLE MEMORY FILE */
mf_free(hfl);									/* WFD FREE MEMORY ALLOCATED FOR FILE */
HelpFlag = OFF;
return(1);										/* RETURN TO FORM EDITOR W. NO ERRORS */
}
/*---------------------------------------------------------------------------
**
**  QUIT PROGRAM KEY
**
*/
int QuitProgramKey()
{
fld_setexit(EXIT_FORM, CURFLDNUM);	/* WFC SET FOR FORM EXIT UPON RETURN */
ExitFlag=ON;
return(1);
}
/*---------------------------------------------------------------------------
**
**   CLEAR ITEM KEY
**
*/
int ClearItemKey()
{
CurRow = CURSCFMRW;
FuncFlag = CLEAR_ITEM;
fld_setexit(EXIT_FORM, CURFLDNUM);	/* WFC SET FOR FORM EXIT UPON RETURN */
return(1);
}
/*---------------------------------------------------------------------------
**
**  DELETE OPTION
**
**
*/
int DeleteOption()
{
WINDOW wn;
int k=0;
int cnt;

defs_wn(&wn, 16, 0, v_rwq-16, v_coq, BDR_DLNP);
sw_popup(ON, &wn);
sw_name(" REMOVE SINGLE MENU OPTION ", &wn);
sw_namelocation(TOPCENTER, &wn);
sw_att(LDOS, &wn);
sw_bdratt(LDOS, &wn);

set_wn(&wn);
csr_hide();
v_plst(1,CENTER_TEXT, "WARNING!", &wn);
v_plst(2,CENTER_TEXT, "THE CURRENT MENU ITEM WILL BE PERMANENTLY ERASED.", &wn);
v_plst(3,CENTER_TEXT, "PRESS THE <Y> KEY TO CONFIRM THIS OPERATION", &wn);
v_plst(4,CENTER_TEXT, "OR", &wn);
v_plst(5,CENTER_TEXT, "PRESS ANY OTHER KEY TO ABORT.", &wn);
k=ki();										/* WFD READ A SINGLE KEY INPUT */
unset_wn(&wn);								/* REMOVE THE MESSAGE WINDOW */
if (k == 89 || k == 121)				/* IF THE KEY INPUT WAS A Y OR A y */
	{											/* THEN THE OPTION SHOULD BE REMOVED FROM THE DATA LIST */
	strcpy(ProcessName[CurRow], "");	/* BLANK THE CURRENT DATA ITEM */
	for (cnt=CurRow;cnt<(MAXMENUITEM-1);cnt++)		/* LOOP FROM THE CURRENT ITEM TO THE END OF THE LIST */
		{															/* TO MOVE ITEM UP IN THE ARRAY */
		strcpy(ProcessName[cnt], ProcessName[cnt+1]);/* STD STORE NEXT PROCESS NAME */
		strcpy(DirPath[cnt], DirPath[cnt+1]);			/* STD STORE NEXT DIR PATH */	
		strcpy(ExecName[cnt], ExecName[cnt+1]);		/* STD STORE NEXT EXEC NAME */
		strcpy(ExecComment[cnt], ExecComment[cnt+1]);/* STD STORE NEXT COMMENT */
		}
	strcpy(ProcessName[cnt], "");
	strcpy(DirPath[cnt], "");
	strcpy(ExecName[cnt], "");
	strcpy(ExecComment[cnt], "");
	}
SaveFile();							/* INT SAVE CURRENT DATA FILE */
return(1);
}
/*---------------------------------------------------------------------------
**
**  LAUNCH
**
**  PURPOSE:
**   THIS FUNCTION IS IMPLEMENTED AS A WFC "KEY FUNCTION" TO PROCESS
**   SELECTION OF THE DESIRED MENU OPTION.  IT WILL SAVE THE NUMBER
**   OF THE CURRENT ROW OF THE SCROLLABLE SUBFORM AND PROCESS A FORM
**   EXIT AT THAT CURRENT ROW.
**
*/
int Launch()
{
FuncFlag=LAUNCH_ITEM;
ProcNum = CURSCFMRW;						/* WFC GET GLOBAL SCROLLABLE FORM ROW */
fld_setexit(EXIT_FORM, CURFLDNUM);	/* WFC SET FOR FORM EXIT UPON RETURN */
return(1);									/* WFC - RETURN FROM KEYFUNCTION W. NO ERROR */
}
/*---------------------------------------------------------------------------
**
**  DISPLAY SERIAL
**
*/
int DisplaySerial()
{
FlagError("Serial #: 01-00002"); /* PLACE THIS MESSAGE IN WINDOW */
return(1);                       /* RETURN FROM KEY FUNCTION - NO ERRORS */
}
/*---------------------------------------------------------------------------
**
**  SET MESSAGE
**
**
*/
int SetMessage()
{
DFORMPTR MsgForm;
DFIELDPTR fp;
MsgForm = fm_def(v_rwq-15, 0, 15, 80, LDOS, BDR_DLNP);
sw_name(" DEFINE SCREEN SAVE MESSAGE ", MsgForm->wnp);
sw_namelocation(TOPCENTER, MsgForm->wnp);
sw_popup(ON, MsgForm->wnp);
sw_bdratt(LDOS, MsgForm->wnp); 
sfm_opt(VERIFYEXIT+AUTOEXIT, OFF, MsgForm);
fp = fld_def(1, 1, "MESSAGE LINE 1:", FBELOW, def_pic('X', 23), F_STRING, UserMessage1, MsgForm);
sf_atts(LREVERSE, LFIELDA, fp);
fp = fld_def(4, 1, "MESSAGE LINE 2:", FBELOW, def_pic('X', 23), F_STRING, UserMessage2, MsgForm);
sf_atts(LREVERSE, LFIELDA, fp);
ftxt_def(12, CENTER_TEXT, "Press <Esc> to exit.", LDOS, MsgForm);
keyd_def(K_ESC, kdexit);/* WFD DEFINE ESC KEY TO EXIT FORM */
fm_proc(0, MsgForm);		/* WFD PROCESS SCREEN SAVER MESSAGES DATA ENTRY FORM */
keyd_def(K_ESC, QuitProgramKey); /* WFD DEFINE ESC KEY TO INTERNAL EXIT ROUTINE */
return(1);					
}
/*---------------------------------------------------------------------------
**
**   CHECK SCREEN
**
**   PURPOSE:
**    THIS FUNCTION WILL INCREMENT A GLOBAL TIMER BETWEEN KEYSTROKES, UNTIL
**    A SPECIFIED DELAY IS REACHED.  AT THAT TIME, THIS FUNCTION WILL CALL
**    A FUNCTION TO PROCESS A SCREEN SAVER ROUTINE AND THEN RESET THE TIMER.
**    IF A KEY HAS BEEN PRESSED, THE TIMER IS RESET TO 0.
**
**   NOTES:
**    THIS FUNCTION IS IMPLEMENTED AS A "KEYLOOP" FUNCTION IN MAIN().
*/    
int CheckScreen()
{
long newtime;												/* CURRENT SYSTEM TIME TO COMPARE WITH OLD SYSTEM TIME FOR SCREEN SAVE */
char CurrentTime[14];									/* CURRENT SYSTEM TIME FORMATTED FOR SCREEN OUTPUT */
int hours;													/* PLACEHOLDER FOR HOURS PORTION OF SYSTEM TIME */
int minutes;												/* PLACEHOLDER FOR MINUTES PORTION OF SYSTEM TIME */
int seconds;												/* PLACEHOLDER FOR SECONDS PORTION OF SYSTEM TIME */
if (ki_chk()) 												/* WFC IF A KEY IS IN THE KEYBOARD BUFFER */
	{															/* THEN THE USER IS AT THE CONSOLE */
	time(&LastKeyTime);      							/* STD GET THE CURRENT SYSTEM TIME AS BEING CURRENT */	
	}
time(&newtime);											/* STD GET THE SYSTEM TIME TO COMPARE WITH LAST KEY PRESSED TIME */
if (((newtime-LastKeyTime) > (long)300) &&      /* IF THE TIMER EXCEEDES THIS DELAY PERIOD */
   (!ScreenSaveFlag))         						/* AND THE SCREEN SAVE FLAG IS NOT ACTIVE */
	{															/* THEN A KEY HAS NOT BEEN PRESSED IN QUITE SOME TIME */
	ScreenSaveFlag=ON;									/* SET SCREEN SAVE FLAG TO AVOID RECURSIVE CALLS */
	ScreenSaver();											/* INT CALL SCREEN SAVER ROUTINE TO KILL TIME */
	ScreenSaveFlag=OFF;									/* CLEAR SCREEN SAVE FLAG */
	time(&LastKeyTime);      							/* STD GET LAST KEY PRESSED TIME AS BEING WHEN SCREEN SAVE QUIT */		
	}								
if ((!ScreenSaveFlag) &&								/* IF THE SCREEN SAVE FLAG IS NOT ACTIVE */
	 (!HelpFlag))
	{															/* THEN UPDATE THE TIME OF DAY ON THE SCREEN */
	get_time(CurrentTime);								/* WFC GET FORMATTED SYSTEM TIME - 24 HOUR CLOCK */
	inttime(CurrentTime, &seconds, &minutes, &hours);	/* WFC BREAK TIME INTO INTEGER COMPONENTS */
	if (hours > 12) 										/* IF THE HOURS PORTION IS AFTER NOON */
		{														/* THEN ADJUST THE 24 HOUR CLOCK TO 12 HOUR CLOCK */
		hours-=12;											/* SIMPLY SUBTRACT 12 FROM THE 24 HOUR CLOCK */
		}
	sprintf(CurrentTime, "%2.2d:%2.2d:%2.2d", hours, minutes, seconds); /* FORMAT TIME FOR DISPLAY */
	v_plst(1, 65, CurrentTime, &MainWindow);		/* WFC DISPLAY THE CURRENT SYSTEM TIME */
	}
return(1);
}
/*---------------------------------------------------------------------------
**
**   SCREEN SAVER
**
**
*/
int ScreenSaver()
{
WINDOW EmptyWindow;											/* WFC WINDOW - USED TO OVERLAY MAIN SCREEN W. BLACK WINDOW */
WINDOW MessageWindow;										/* WFC WINDOW FOR USER MESSAGE DURING SCREEN SAVE */
int rw;															/* UPPER LEFT HAND CORNER OF MESSAGE WINDOW */
int co;															/* UPPER LEFT HAND COLUMN OF MESSAGE WINDOW */
int newtime;
int oldtime;
keyd_def(K_ESC, NULLFP); 									/* WFD DEFINE ESC KEY TO INTERNAL EXIT ROUTINE */
csr_hide();														/* WFC HIDE THE CURSOR */	
defs_wn(&EmptyWindow, 0, 0, v_rwq, v_coq, BDR_0P);	/* WFC DEFINE EMPTY WINDOW */
sw_popup(ON, &EmptyWindow);								/* WFC SET EMPTY WINDOW TO POPUP */
sw_att(LDOS, &EmptyWindow);								/* WFC SET EMPTY WINDOW TO BLACK BACKGROUND */
defs_wn(&MessageWindow, CENTER_WN, CENTER_WN, 6, 25, BDR_LNP);/* WFC DEFINE MESSAGE WINDOW */
sw_popup(ON, &MessageWindow);								/* WFC SET MESSAGE WINDOW TO POPUP */
sw_name(" MACRO-MENU ", &MessageWindow);			/* WFC SET MESSAGE WINDOW TITLE */
sw_namelocation(TOPCENTER, &MessageWindow);			/* WFC SET MESSAGE WINDOW TITLE LOCATION */
set_wn(&EmptyWindow);										/* WFC SET EMPTY WINDOW ON DISPLAY */
while(!ki_chk())												/* WFC WHILE A KEY HAS NOT BEEN PRESSED */
	{																/* PERFORM SCREEN SAVE LOOP */	
	set_wn(&MessageWindow);									/* WFC PLACE MESSAGE WINDOW ON DISPLAY */
	v_plst(0, CENTER_TEXT, UserMessage1, &MessageWindow);
	v_plst(1, CENTER_TEXT, UserMessage2, &MessageWindow);
	v_plst(3, CENTER_TEXT, "press any key", &MessageWindow);
	time(&oldtime);
	time(&newtime);
	while(newtime-oldtime < 15) 
		{
		if (ki_chk()) break;
		time(&newtime);
		}
	unset_wn(&MessageWindow);								/* WFC REMOVE THE MESSAGE WINDOW FROM DISPLAY */
	rw=(int) (((double)rand()/32767)*100)&0x11;		/* STD GET RANDOM UPPER-LEFT ROW COORDINATE (24 = 18hex) */
	co=(int) (((double)rand()/32767)*100)&0x37;		/* STD GET RANDOM UPPER-LEFT COLUMN COORDINATE (80 = 50hex) */
	mod_wn(rw, co, 6, 25, &MessageWindow);				/* WFC MODIFY WINDOW COORDINATES */
	}
ki();																/* WFC PULL KEYSTROKE FROM BUFFER */
if (isset_wn(&MessageWindow))
	{
	unset_wn(&MessageWindow);								/* WFC REMOVE THE MESSAGE WINDOW FROM THE DISPLAY */
	}
unset_wn(&EmptyWindow);										/* WFC REMOVE THE EMPTY WINDOW FROM THE DISPLAY */
keyd_def(K_ESC, QuitProgramKey); 						/* WFD DEFINE ESC KEY TO INTERNAL EXIT ROUTINE */
csr_show();														/* WFC SHOW THE CURSOR */
return(0);														/* RETURN FROM SCREEN SAVE ROUTINE W. NO ERRORS */
}
/*---------------------------------------------------------------------------
**
**  FLAG ERROR
**
**
*/
int FlagError(msg)
char *msg;
{
WINDOW ewn;
defs_wn(&ewn, 19, 0, 6, v_coq, BDR_LNP);
sw_popup(ON, &ewn);
sw_att(LREVERSE, &ewn);
sw_bdratt(LREVERSE, &ewn);
sw_name(" MESSAGE ", &ewn);
sw_namelocation(TOPCENTER, &ewn);
csr_hide();
set_wn(&ewn);
v_printf(&ewn, "%s\n\nPress any key to continue.", msg);
ki();
unset_wn(&ewn);
csr_show();
return(1);
}

