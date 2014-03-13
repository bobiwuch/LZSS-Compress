/*Robert Iwuchukwu*/
/*bobiwuch@brandeis.edu*/

#include <stdio.h>
#include <stdlib.h>

#define DEF_WINDOWSIZE 4096
#define DEF_HASHSIZE 1024
#define LOOKAHEAD 16


typedef struct win{
	unsigned char window[DEF_WINDOWSIZE];
	int lookaheadFront, lookaheadRear;
	int numInDict;
} Win;
typedef Win *window;

typedef struct listStruc{
	struct listStruc *next;
	int windowPosition;
	int lengthOfMatch;
} listS;
typedef listS *list;

typedef struct hashish{
	list hTable[DEF_HASHSIZE];
} Hash;
typedef Hash *hashType;

typedef struct bFile{
	unsigned char mask;  
	int onDeckByte;
	FILE *filestream;
	int numBytesWritten;
} BFile;
typedef BFile *BFILE;

typedef struct outFile{
	unsigned char mask;  
	int onDeckByte;
	FILE *filestream;
	int numBytesWritten;
} BoutFile;
typedef BoutFile *OUTFILE;

BFILE openFile(char *filename){
	BFILE f = (BFILE) malloc(sizeof(BFile));
	if (f==NULL) {printf("Malloc for a new file failed."); exit(1);}
	
	f->filestream = fopen(filename, "rb");
	f->numBytesWritten = 0;
	f->mask = 0x80;
	f->onDeckByte = 0;
	return f;
}

OUTFILE openOutFile(char *filename){
	OUTFILE f = (OUTFILE) malloc(sizeof(BoutFile));
	if (f==NULL) {printf("Malloc for a new file failed."); exit(1);}
	
	f->filestream = fopen(filename, "wb+");
	f->numBytesWritten = 0;
	f->mask = 0x80;
	f->onDeckByte = 0;
	return f;
}

void closeFile(BFILE f){
	int test = fclose(f->filestream);
	if(test == 1)
		printf("ERROR CLOSING FILE");
	free(f);
}

void closeOutFile(OUTFILE f){
	int test = fclose(f->filestream);
	if(test == 1)
		printf("ERROR CLOSING FILE");
	free(f);
}

int getBit(BFILE f){
	/*If the mask is equal to the starting value, we are at an even byte 
	locale. Accordingly, check all the possible states of an even byte position.
	If not at the end of the file, get a new byte and store it 
	in the onDeckByte. 'bit' is the value of the most significant current bit
	in the file.*/
	if(f->mask == 0x80){
		f->onDeckByte = fgetc(f->filestream);
		if(f->onDeckByte == EOF){
			printf("\n\nEND OF FILE ENCOUNTERED IN GETBIT\n");
			return -1;
		}
		f->numBytesWritten++;
		if(f->numBytesWritten %3072 == 0)
			printf("*");
	}
	
	int bit = f->onDeckByte & f->mask;
	f->mask >>= 1;
	
	if(f->mask == 0){
		f->mask = 0x80;
	}
	
	if(bit)
		return 1;
	else
		return 0;
}

unsigned long getBits(BFILE f, int lengthOfBits){
	unsigned long localMask;
	unsigned long gottenBits = 0;

	localMask = 1;
	localMask <<= lengthOfBits-1;
	
	while(localMask != 0){
		if(f->mask == 0x80){
			f->onDeckByte = fgetc(f->filestream);
			if(f->onDeckByte == EOF){
				printf("\n\nEND OF FILE ENCOUNTERED IN GETBITS\n");
				return -1;
			}
			f->numBytesWritten++;
			if(f->numBytesWritten %3072 == 0)
				printf("*");
		}
		if(f->onDeckByte & f->mask)
			gottenBits |= localMask;
		
		localMask >>= 1;
		f->mask >>= 1;
		if(f->mask == 0){
			f->mask = 0x80;
		}
	}
	return gottenBits;
}

void sendBit(OUTFILE f, int bit){
	/*If writing an on (1) bit to the onDeckByte, OR (|) the mask to the 
	onDeckByte and store it in onDeckByte. Regardless of the bit being
	written, shift the mask to the right by one after each call. After 8 calls
	the mask will be 0; now write the onDeckByte to the OUTFILE*/
	
	if(bit){
		f->onDeckByte |= f->mask;
	}
	f->mask >>= 1;
	if(f->mask == 0){
		putc(f->onDeckByte, f->filestream);
		f->numBytesWritten++;
		if(f->numBytesWritten %3072 == 0)
			printf("*");
		f->mask = 0x80;
		f->onDeckByte = 0;
	}		
}

void sendBits(OUTFILE f, unsigned long bits, int lengthOfBits){
	unsigned long localMask;
	
	localMask = 1;
	localMask <<= lengthOfBits-1;
	
	while(localMask != 0){
		if(bits & localMask)
			f->onDeckByte |= f->mask;
		f->mask >>= 1;
		
		if(f->mask == 0){
			putc(f->onDeckByte, f->filestream);
			f->numBytesWritten++;
			if(f->numBytesWritten %3072 == 0)
				printf("*");
			f->mask = 0x80;
			f->onDeckByte = 0;
		}
		localMask >>= 1;
	}
}

window initWindow(){
	window x = (window) malloc(sizeof(Win));
	x->lookaheadFront = 0;
	x->lookaheadRear = 0;
	x->numInDict = 0;
	return x;
}

hashType initHash(){
	int i;
	hashType h = (hashType) malloc(sizeof(Hash));
	for(i=0; i<DEF_HASHSIZE;i++){
		h->hTable[i] = NULL;
	}
	return h;
}

void clearHash(hashType h){
	int i=0;
	for(i=0; i<DEF_HASHSIZE;i++){
		if(h->hTable[i] != NULL){
			
			while(h->hTable[i] != NULL){
				list n = h->hTable[i];
				h->hTable[i] = h->hTable[i]->next;
				free(n);
			}
		}
	}
}

int loadLookahead(window x, FILE *f){
	int i=0;
	int c;
	
	while (i < LOOKAHEAD) {
		c = fgetc(f);
		/*
		if(c == EOF)
			break;
		*/
		x->window[x->lookaheadRear] = c;
		x->lookaheadRear = (x->lookaheadRear+1) % DEF_WINDOWSIZE;
		i++;
		
		
	}
	if(feof(f)){
		printf("\n\nEND OF FILE ENCOUNTERED IN LOADLOOKAHEAD\n");
	}
	return c;
}

int powr(int x, int y){
	if(y==0)
		return 1;
	else
		return x*powr(x,y-1);
}

int hash(unsigned char *array, int position){
	unsigned int i, result=0;
	
	for(i=0;i<2;i++){
		result += (array[ (i + position) % DEF_WINDOWSIZE ]*powr(13,i+1)) % DEF_HASHSIZE;
	}
	return result % DEF_HASHSIZE;	
}

list newListNode(list l, int position){
	list nl = (list) malloc(sizeof(listS));
	nl->next = l;
	nl->windowPosition = position;
	
	return nl;	
}

void hashDictionary(hashType h, window x){
	int hashVal;
	int i, val;
	
	clearHash(h);
	
	if(x->numInDict < DEF_WINDOWSIZE - LOOKAHEAD){
		for(i=0; i < x->numInDict-2; i++){
			hashVal = hash(x->window, i);
			
			if(h->hTable[hashVal] == NULL){
				h->hTable[hashVal] = newListNode(NULL, i);
			}
			else{
				h->hTable[hashVal] = newListNode(h->hTable[hashVal], i);
			}	
		}
	
	}
	
	
	else{
		i = x->lookaheadRear;
		val = x->lookaheadFront -2;
		
		if(val < 0){
			if(val == -1)
				val = DEF_WINDOWSIZE - 1;
			else
				val = DEF_WINDOWSIZE - 2;
		}
		
		while( i != val ){	
			hashVal = hash(x->window, i);
			
			if(h->hTable[hashVal] == NULL){
				h->hTable[hashVal] = newListNode(NULL, i);
			}
			else{
				h->hTable[hashVal] = newListNode(h->hTable[hashVal], i);
			}
			i = (i+1) % DEF_WINDOWSIZE;			
		}
	}
}

/*Given the window, and a list of possible longest match locations, find the
best match and store it and the length of the match in plArray(position,length)*/
void getBestMatch(window x, list l, int *plArray){
	int lengthOfMatch, bestMatchLength = 0;
	int i, distance, lookahead;
	int iterPosition;
	
	distance = 0;
	int laf = x->lookaheadFront;
	int lar = x->lookaheadRear;
	
	
	if(l == NULL){
		plArray[0] = 0;
		plArray[1] = 0;
	}
	else{
		if(lar >= laf)
			distance = lar-laf;
		else{
			distance = DEF_WINDOWSIZE - laf;
		}
		list dummy = l;
		
		while(dummy != NULL){
			lengthOfMatch = 0;
			iterPosition = dummy->windowPosition;
			lookahead = x->lookaheadFront;
			for(i = 0; i < distance; i++){
				if(x->window[iterPosition] == x->window[lookahead]){
					lengthOfMatch++;
					iterPosition = (iterPosition + 1) % DEF_WINDOWSIZE;
					lookahead = (lookahead + 1) % DEF_WINDOWSIZE;
				}
				else 
					break;
			}
			if(lengthOfMatch >= bestMatchLength){
				plArray[0] = dummy->windowPosition;
				plArray[1] = lengthOfMatch;
				bestMatchLength = lengthOfMatch;
			}
			dummy = dummy->next;
		}
	}
}

void updateHash(hashType h, window x, int oldFront, int matchLength){
	int i, hashVal;

	for(i=0; i<matchLength-1; i++){
		hashVal = hash(x->window, i + oldFront);
		
		if(h->hTable[hashVal] == NULL){
			h->hTable[hashVal] = newListNode(NULL, (i + oldFront) % DEF_WINDOWSIZE);
		}
		else{
			h->hTable[hashVal] = newListNode(h->hTable[hashVal], (i + oldFront) % DEF_WINDOWSIZE);
		}
		
		hashVal = hash(x->window, i + x->lookaheadRear);
		
		if(h->hTable[hashVal] != NULL){
			while(h->hTable[hashVal] != NULL){
				list d = h->hTable[hashVal];
				h->hTable[hashVal] = h->hTable[hashVal]->next;
				free(d);
			}
		}
		
	}
}

void codeLookahead(window x, hashType h, OUTFILE f){
	int pl[2];
	int hashVal;
	int matchLength, matchPosition, ola;
	
	hashDictionary(h, x);
	
	while(x->lookaheadFront != x->lookaheadRear){  
		pl[0] = 0;
		pl[1] = 0;
		
		/*Get the longest match length and its position.*/
		hashVal = hash(x->window, x->lookaheadFront);
		list l = h->hTable[hashVal];
		getBestMatch(x, l, pl);
		matchPosition = pl[0];
		matchLength = pl[1];
		
		/*If there is no equal hash, output a 0-bit and then the current character
		otherwise output a 1-bit and 12-bits for the position, and 4 bits for the 
		length of the match*/
		if(matchLength < 2){
			sendBit(f,0);
			sendBits(f, x->window[x->lookaheadFront], 8);
			ola = x->lookaheadFront;
			x->lookaheadFront = (x->lookaheadFront+1) % (DEF_WINDOWSIZE);
			x->numInDict = x->numInDict +1;
		}
		else{
			sendBit(f,1);
			if(matchLength == 16)
				matchLength = 15;
			sendBits(f, matchPosition, 12);
			sendBits(f, matchLength, 4);
			ola = x->lookaheadFront;
			x->lookaheadFront = (x->lookaheadFront + matchLength) % (DEF_WINDOWSIZE);
			x->numInDict += matchLength;
		}
		
		updateHash(h, x, ola, matchLength);
	}
}

void codeLookaheadFixed(window x, hashType h, OUTFILE f){
	int pl[2];
	int hashVal;
	int matchLength, matchPosition, ola;
	
	hashDictionary(h, x);
	
	while(x->lookaheadFront != x->lookaheadRear){  
		pl[0] = 0;
		pl[1] = 0;
		
		/*Get the longest match length and its position.*/
		hashVal = hash(x->window, x->lookaheadFront);
		list l = h->hTable[hashVal];
		getBestMatch(x, l, pl);
		matchPosition = pl[0];
		matchLength = pl[1];
		
		/*If there is no equal hash, output a 0-bit and then the current character
		otherwise output a 1-bit and 12-bits for the position, and 4 bits for the 
		length of the match*/
		if(matchLength < 2){
			sendBits(f, x->window[x->lookaheadFront], 16);
			ola = x->lookaheadFront;
			x->lookaheadFront = (x->lookaheadFront+1) % (DEF_WINDOWSIZE);
			x->numInDict = x->numInDict +1;
		}
		else{
			if(matchLength == 16)
				matchLength = 15;
			sendBits(f, matchLength, 4);
			sendBits(f, matchPosition, 12);
			ola = x->lookaheadFront;
			x->lookaheadFront = (x->lookaheadFront + matchLength) % (DEF_WINDOWSIZE);
			x->numInDict += matchLength;
		}
		
		updateHash(h, x, ola, matchLength);
	}
}

/*From x---->y ; If toggle is 0, code fixed else code variable */
void compress(FILE *x, OUTFILE y, int toggle){
	char c;
	
	window w = initWindow();
	hashType h = initHash();
	
	if(toggle == 0){
		do{
			c = loadLookahead(w, x);
			codeLookaheadFixed(w, h, y);
		}while(c > 0);
	}
	else{
		do{
			c = loadLookahead(w, x);
			codeLookahead(w, h, y);
		}while(c > 0);
	}
	
	fclose(x);
	closeOutFile(y);
}


/*From x---->y  */
void decompress(BFILE x, OUTFILE y){
	int currentBit, i, indexOfMatch, lengthOfMatch;
	int currChar;
	window w = initWindow();
	
	printf("\n\nSTART DECOMP WHILE LOOP\n\n");
	while((currentBit = getBit(x)) != -1){
		
		if(currentBit == 0){
			currChar = getBits(x,8);
			
			if(currChar == 255)
				break;
			
			w->window[w->lookaheadRear] = currChar;
			w->lookaheadRear = (w->lookaheadRear+1) % DEF_WINDOWSIZE;
			
			sendBits(y, currChar, 8);
		}
		else{
			indexOfMatch = getBits(x,12);
			lengthOfMatch = getBits(x,4);
			
			if(indexOfMatch < 0 || lengthOfMatch < 0)
				break;
			
			for(i=0;i<lengthOfMatch;i++){
				currChar = 	w->window[(indexOfMatch + i) % DEF_WINDOWSIZE];
				
				if(currChar == 255)
					break;
			
				w->window[w->lookaheadRear] = currChar;
				w->lookaheadRear = (w->lookaheadRear+1) % DEF_WINDOWSIZE;
				
				sendBits(y, currChar, 8);
			}
			
		}
	}
	printf("\n\nEND DECOMP WHILE LOOP\n\n");
	
	closeFile(x);
	closeOutFile(y);
	
}

/*From x---->y ; decompresser for fixed length pointers */
void decompressFixed(BFILE x, OUTFILE y){
	long current16;
	int i;
	unsigned int indexOfMatch, lengthOfMatch;
	int currChar;
	window w = initWindow();
	
	printf("\n\nSTART DECOMP WHILE LOOP\n\n");
	while((current16 = getBits(x,16)) != -1){
		
		if(current16 < 255){
			currChar = current16;
			
			if(currChar < 0)
				break;
			
			w->window[w->lookaheadRear] = currChar;
			w->lookaheadRear = (w->lookaheadRear+1) % DEF_WINDOWSIZE;
			
			sendBits(y, currChar, 8);
		}
		else{
			/*Get the coded values in the current16*/
			lengthOfMatch = (current16 & 0xF000) >> 12;
			indexOfMatch = (current16 & 0x0FFF);
			
			
			for(i=0;i<lengthOfMatch;i++){
				currChar = 	w->window[(indexOfMatch + i) % DEF_WINDOWSIZE];
				
				if(currChar < 0)
					break;
			
				w->window[w->lookaheadRear] = currChar;
				w->lookaheadRear = (w->lookaheadRear+1) % DEF_WINDOWSIZE;
				
				sendBits(y, currChar, 8);
			}
			
		}
	}
	printf("\n\nEND DECOMP WHILE LOOP\n\n");
	
	closeFile(x);
	closeOutFile(y);
	
}
