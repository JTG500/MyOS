//Eric Foster
void printString(char*);
void readString(char*);
void readSector(char*, int);
void readFile(char*, char*);
void executeProgram(char*, int);
void terminate();
int mod(int, int);
int div(int, int);
void handleInterrupt(int, int, int, int);
void main2();

int main()
{
main2();
}

typedef struct
{
	//int segment;
	int active;
	int stack;
}ProcessTable;

ProcessTable process[8];
int currentProcess;

void main2()
{
	int i;
	makeInterrupt21();
	for (i = 0; i < 8; i++)
	{
		//process[i].segment = (0x2+i) * (0x1000);
		process[i].active = 0;
		process[i].stack = 0xff00;
	}
	currentProcess = -1;
	executeProgram("shell\0");
	makeTimerInterrupt();
}

int getSegment(int index)
{
	int segment;
	segment = (0x2+index) * 0x1000;
	return segment;
}

int emptySegment()
{
	int i;
	int actualSegment = setKernelDataSegment();
	for (i = 0; i < 8; i++)
	{
		if (process[i].active == 0)
		{
			restoreDataSegment(actualSegment);
			return i;
		}
	}
}

void printString(char* phrase)
{
	int i;
	for (i = 0; phrase[i] != 0x0; i++)
	{
		interrupt(0x10,0xe*256+phrase[i],0,0,0);
	}
}

void readString(char* line)
{
	int i = 0, final;
	char character = 0x0;
	while (character != 0xd)
	{	
		character = interrupt(0x16,0,0,0,0);
	
		if (character == 0x8)
		{
			if (i > 0)
			{
				interrupt(0x10,0xe*256+0x8,0,0,0);
				interrupt(0x10,0xe*256+0x0,0,0,0);
				interrupt(0x10,0xe*256+0x8,0,0,0);
				i--;
			}
		}
		else
		{
			if (character != 0xd)
			{
				line[i] = character;
				interrupt(0x10,0xe*256+line[i],0,0,0);
				if (i < 78)
				{
					i++;
				}
			}
		}
		
		
	}
	
	line[i] = 0x0;
	interrupt(0x10,0xe*256+'\r',0,0,0);
	interrupt(0x10,0xe*256+'\n',0,0,0);
}

void readSector(char* buffer, int sector)
{
	int relativeSector;
	int head;
	int track;
	int temp;

	relativeSector = mod(sector,18) + 1;
	temp = div(sector,18);
	head = mod(temp,2);
	track = div(sector,36);

interrupt(0x13,2*256+1,buffer,track*256+relativeSector,head*256+0);
}

int mod(int a,int b)
{
	while(a>=b)
	{
		a=a-b;
	}
	return a;
}

int div(int a,int b)
{
	int q = 0;
	while(q*b <= a)
	{
		q=q+1;
	}
	return q-1;
}	

void readFile(char* fileName, char* buffer)
{
	int i, j, k, bufferAddress = 0, match = -1, index, count = 0;
	int sectors[27];
	readSector(buffer, 2);
	for(i = 0; i < 16; i++)
	{
		if (buffer[32*i] != 0x0)
		{
			count = 0;
			for (j = 0; j < 6; j++)
			{
				
				if (buffer[j+32*i] == fileName[j])
				{
					count++;
					if (count == 6)
						match = i;
				}
			}
		}
	}

	if (match == -1)
	{
		printString("File not Found");
		return;
	}
	else
	{
		index = match*32+6;
		sectors[26] = 0;
		for (k = 0; k < 26; k++)
		{
			sectors[k] = buffer[index+k];
		}

		for (k = 0; sectors[k] != 0x0; k++)
		{
			readSector(buffer+bufferAddress,sectors[k]);
			bufferAddress = bufferAddress + 512;
		}	

	}
}

/*void exeProgram(char* name, int segment)
{
	int i = 0;
	char buffer[13312];
	readFile(name, buffer);

		for (i = 0; i < 13312; i++)
		{
			putInMemory(segment,i,buffer[i]);
		}
		launchProgram(segment);
}*/

void executeProgram(char* name)
{
	int i = 0, j = 0, actualSegment;
	char buffer[13312];
	int empty = emptySegment();
	int segment = getSegment(empty);
	readFile(name, buffer);
	for (j = 0; j < 13312; j++)
	{
		putInMemory(segment,j,buffer[j]);
	}
	actualSegment = setKernelDataSegment();
	process[empty].active = 1;
	process[empty].stack = 0xff00;
	restoreDataSegment(actualSegment);
	initializeProgram(segment);
}
void terminate()
{
	int actualSegment = setKernelDataSegment();
	process[currentProcess].active = 0;
	while(1);
	restoreDataSegment(actualSegment);
}

void writeSector(char* buffer, int sector)
{
	int relativeSector;
	int head;
	int track;
	int temp;

	relativeSector = mod(sector,18) + 1;
	temp = div(sector,18);
	head = mod(temp,2);
	track = div(sector,36);

interrupt(0x13,3*256+1,buffer,track*256+relativeSector,head*256+0);
}

void deleteFile(char* name)
{
	char map[512];
	char dir[512];
	int sectors[27];
	int match = -1, i = 0, j = 0, k = 0, index, currentSector, count = 0;
	readSector(map, 1);
	readSector(dir, 2);
	for(i = 0; i < 16; i++)
	{
		if (dir[32*i] != 0x0)
		{
			count = 0;
			for (j = 0; j < 6; j++)
			{
				
				if (dir[j+32*i] == name[j])
				{
					count++;
					if (count == 6)
						match = i;
				}
			}
		}
	}
	if (match == -1)
	{
		printString("File not Found");
		return;
	}

	for (i = 0; i < 6; i++)
	{
		dir[match*32+i] = 0x00;
	}
	index = match*32+6;
		sectors[26] = 0;
		for (k = 0; k < 26; k++)
		{
			sectors[k] = dir[index+k];
			dir[index+k] = 0x00;
		}

	index = match*32+6;
		
		for (k = 0; k < 26; k++)
		{
			sectors[k] = dir[index+k];
			
		}

		sectors[26] = 0x0;
		
		for (k = 0; k < 26; k++)
		{
			currentSector = sectors[k];
			if (currentSector == 0)
			{
				break;
			}		
			map[currentSector] = 0x00;
		}
		
		writeSector(map, 1);
		writeSector(dir, 2);	
}

void writeFile(char* name, char* buffer, int numberOfSectors)
{
	char map[512];
	char dir[512];
	int dirContents;
	int available = 0;
	int count = 0;
	int i, j;
	int sectorNumber = 0;
	char fileContents[512];
	int whichSector;

	readSector(map, 1);
	readSector(dir, 2);
	
	for (dirContents = 0; dirContents < 16; dirContents++)
	{
		if (dir[32*dirContents] == 0x00)
		{
			available = 1;
			break;
		}
	}
	if (available == 0)
	{
		return;
	}

	while (name[count] != '\0' && name[count] != 0x0)
	{
		count++;
	}
	
	for (i = 0; i < count; i++)
	{
		dir[32*dirContents+i] = name[i];
	}
	if (count < 6)
	{
		for (i = count; i < 6; i++)
		{
			dir[32*dirContents+i] = 0x0;
		}
	}
	for (i = 0; i < numberOfSectors; i++)
	{
		sectorNumber = 4;
		while (map[sectorNumber] != 0x00)
		{
			sectorNumber++;
		}
		if (sectorNumber >= 26)
		{
			return;
		}
		map[sectorNumber] = 0xFF;
		dir[32*dirContents+6+i] = sectorNumber;
		for (j = 0; j < 512; j++)
		{
			whichSector = i+1;
			fileContents[j] = buffer[j*whichSector];
		}
		writeSector(fileContents, sectorNumber);
	}

	writeSector(map, 1);
	writeSector(dir, 2);
}

void killProcess(int processID)
{
	int actualSegment;
	actualSegment = setKernelDataSegment();
	process[processID].active = 0;
	restoreDataSegment(actualSegment);
}



void handleInterrupt21(int ax, int bx, int cx, int dx)
{
	if (ax == 0)
	printString(bx);

	if (ax == 1)
	readString(bx);

	if (ax == 2)
	readSector(bx, cx);

	if (ax == 3)
	readFile(bx, cx);

	if (ax == 4)
	executeProgram(bx, cx);

	if (ax == 5)
	terminate();

	if (ax == 6)
	writeSector(bx, cx);

	if (ax == 7)
	deleteFile(bx);
	
	if (ax == 8)
	writeFile(bx, cx, dx);

	if (ax == 9)
	killProcess(bx);
}



void handleTimerInterrupt(int segment, int sp)
{
		int i, x, inactiveCount = 0, currentSegment, actualSegment;
		int getProcess = (segment/0x1000) - 2;
		actualSegment = setKernelDataSegment();
		if (getProcess != -1)
			process[getProcess].stack = sp;
		for (i = 0; i < 8; i++)
		{
			if (process[i].active == 0)
				inactiveCount++;
		}
		if (inactiveCount == 8)
		{
			returnFromTimer(segment, sp);
		}
		else
		{
			for (x = 0; x < 8; x++)
			{
				i = mod(x + currentProcess + 1, 8);
				if (process[i].active == 1)
				{
					currentProcess = i;
					break;
				}
			}
			currentSegment = getSegment(currentProcess);
			returnFromTimer(currentSegment, process[currentProcess].stack);
			restoreDataSegment(actualSegment);
		}
}

