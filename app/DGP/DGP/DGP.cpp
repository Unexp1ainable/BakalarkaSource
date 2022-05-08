// DGP.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <io.h>
#include <string.h>
#include <windows.h>
#include <string>

#include "DGP_Defines.h"
#include "DGPObList.h"
#include "DGP_Patch.h"
#include "DGP_FileIO.h"
#include "DGP_Reconstruction.h"


DGP_BOOL fileChosenByList(char directorty[], char filespef2[], char selectedFileName[])
{
    struct _finddata_t c_file;
    intptr_t hFile, fileNum = 0;
    DGP_CHAR filespef[200];
    int colNum = 3, colsize;

    colsize = (60/colNum)-4;
    strcpy(filespef, directorty);
    strcat(filespef, filespef2); 

    if((hFile=_findfirst(filespef, &c_file))==-1L){  
		printf( "No file is found!\n");
        return false;
    }
    printf("%*d: %s %*s", 2, fileNum++, c_file.name, colsize-strlen(c_file.name), " ");
    while(_findnext(hFile, &c_file)!=-1L) {
        printf( "%*d: %s %*s", 2, fileNum++, c_file.name, colsize-strlen(c_file.name), " ");
        if((fileNum%colNum)==0) printf("\n");
    }
    _findclose(hFile);
	printf("\n");

    DGP_INT inputNum;   DGP_CHAR inputStr[200];
    printf("\nPlease select the file name for import: ");
    scanf("%s",inputStr);   printf("\n");   sscanf(inputStr,"%d",&inputNum);
    if((inputNum<0)||(inputNum>=fileNum)) {printf("Incorrect Input!!!\n"); return false;}

    fileNum=0;
    if((hFile=_findfirst(filespef, &c_file))==-1L) {return false;}
    if(inputNum!=0){
        fileNum++;
        while(_findnext( hFile, &c_file )!=-1L){
            if (fileNum==inputNum) break;
            fileNum++;
        }
    }
    _findclose(hFile);
    strcpy(selectedFileName, c_file.name);
    printf("-------------------------------------------------------\nSelected File: %s\n", selectedFileName);
    return true;
}

int _tmain(int argc, _TCHAR* argv[])
{ 
	//step 1: input Model
	DGP_CHAR path_buffer[_MAX_PATH], filename[_MAX_PATH], drive[_MAX_DRIVE], 
		dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT], name[100], surfFlag[100];

	GetModuleFileName(NULL, path_buffer, 260);
	_splitpath(path_buffer, drive, dir, fname, ext);
	strcpy(filename, dir);
	strcat(filename, MODEL_PATH);
	if(!fileChosenByList(filename, "*", name)) return 0;
	strcat(filename, name);
#ifdef OUTPUT_INFO
	clock_t t1 = clock();
#endif
	DGP_Reconstruction* recon = new DGP_Reconstruction;
	DGP_FileIO fileIO;
	DGP_INT partNum = 0;
	if(!fileIO.ReadNormalData(filename, recon->GetPatchList(), recon->GetVertexList(), partNum)){
		printf("File input error, please check the normal file!\n");
		delete recon; recon = NULL; return 0;
	}
#ifdef OUTPUT_INFO
	clock_t t2 = clock();
	printf("File input time: %ld\n", t2-t1);
#endif

	SOLVER_TYPE solve_type = DIRECT_SOLVER;
	DGP_INT maxStep = 5;

	printf("Begin reconstruction... 5 steps\n");
#ifdef OUTPUT_INFO
	t2 = clock();
#endif
	recon->DoComputing(solve_type, partNum, maxStep);
	printf("Finish reconstruction...\n");
#ifdef OUTPUT_INFO
	clock_t t3 = clock();
	printf("Reconstruction time: %ld\n", t3-t2);
#endif
	//step 5: output result
	std::string objname = name;
	objname.replace(objname.end()-3,objname.end(),"obj");
	strcpy(name, objname.c_str());
	bool withSurface = true;
	
	strcpy(filename, dir);
	strcat(filename, "Result\\");
	strcat(filename, name);

	fileIO.WriteResultData(filename, recon->GetPatchList(), recon->GetVertexList(), withSurface);
	printf("Finish output...\n");
	delete recon; recon = NULL;
	return 0;
}

