#include "com_util.h"
#include <string.h>

char *pName;



void help(){
	printf("\n\n\nSyntax is: %s [option] [file_1] [file_2]...\n\n", pName);
	printf("Option:\n");
	printf("\t-f\t\tFIXED LENGTH POINTERS\n\n");
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
			FILE *f = fopen(argv[1], "rb");
			strcpy(str, argv[1]);
			if(fixed == 0)
				strcat(str,".compF");
			else
				strcat(str,".comp");
			OUTFILE of = openOutFile(str);
			compress(f, of, fixed);	
			argc--;
			argv++;
		}
	}
	
	printf("\n\n\nPRESS ENTER TO EXIT\n\n");
	getchar();
	return 0;	
}
