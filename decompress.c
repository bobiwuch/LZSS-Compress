/*Robert Iwuchukwu*/
/*bobiwuch@brandeis.edu*/

#include "com_util.h"
#include <string.h>

char *pName;



void help(){
	printf("\n\n\nSyntax is: %s [option] [file_1] [file_2]...\n\n", pName);
	printf("Option:\n");
	printf("\t-f\t\tFIXED LENGTH POINTER DECOMPRESSION\n\n");
	exit(5);
}

int main(int argc, char *argv[]){
	pName = argv[0];
	int fixed = 1;
	char str[80];
	
	while ((argc > 1) && (argv[1][0] == '-')){
			
		switch(argv[1][1]){
		case 'f':
			fixed = 0;
			break;
		default:
			help();
		}
		argc--;
		argv++;
	}
	
	if(argc == 1){
		help();			
	}
	else{
		while (argc >1){
			BFILE f = openFile(argv[1]);
			strcpy(str, argv[1]);
			if(fixed == 0){
				str[strlen(str)-6] = '\0';
				OUTFILE of = openOutFile(str);
				decompressFixed(f, of);
			}	
			else{
				str[strlen(str)-5] = '\0';
				OUTFILE of = openOutFile(str);
				decompress(f, of);
			}
			
				
			argc--;
			argv++;
		}
	}
		
	printf("\n\n\nPRESS ENTER TO EXIT\n\n");
	getchar();
	return 0;	
}

