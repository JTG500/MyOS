//Eric Foster

int main()
{
	enableInterrupts();
	while(1)
	{
	int i = 0, typeCount = 0, exCount = 0, currentCount = 0, delCount = 0, copyCount = 0, fileLength = 0, sectors = 0, nextCount = 0;
	int count = 0, nameLength = 0, dirCount = 0, createCount=0, killCount = 0;
	int j = 0, internalCount = 0, temp = 0, length = 0, sector = 4;
	int store = 0, index = 0, killer;
	char message[80];
	char input[80];
	char dirName[10];
	char* format = "\r\n";
	char command[80];
	char fileName[80];
	char fileName2[80];
	char buffer[13312];
	char subBuffer[13312];
        char* type = "type";
	char* execute = "execute";
	char* delete = "delete";
	char* copy = "copy";
	char* dir = "dir";
	char* create = "create";
	char* kill = "kill";
	
	for (i = 0; i < 80; i++)
	{
		message[i] = 0x0;
		command[i] = 0x0;
		fileName[i] = 0x0;
		fileName2[i] = 0x0;
		input[i] = 0x0;
	}
	interrupt(0x21,0, "Shell> ",0,0);
	interrupt(0x21,1, message, 0, 0);
	i = 0;
	while (message[i] != ' ' && message[i] != 0x0)
	{
		command[i] = message[i];
		i++;
		currentCount = i+1;
	}
	i = currentCount;
	while ((message[currentCount] != ' ') && nameLength < 6)
	{
		if (nameLength < 6)
		fileName[currentCount-i] = message[currentCount];
		fileName[(currentCount-i)+1] = '\0';
		currentCount++;
		nameLength++;
		nextCount = currentCount+1;
	}
	i = nextCount;
	while ((message[nextCount] != 0x0) || nameLength < 6)
	{
		fileName2[nextCount-i] = message[nextCount];
		fileName2[(nextCount-i)+1] = '\0';
		nextCount++;
		nameLength++;
	}
	for (i = 0; i < 3; i++)
	{
		if (command[i] == dir[i])
		{
			dirCount++;
		}
	}
	for (i = 0; i < 4; i++)
	{
		if (command[i] == type[i])
		{
			typeCount++;
		}
		if (command[i] == copy[i])
		{
			copyCount++;
		}
		if (command[i] == kill[i])
		{
			killCount++;
		}
	}
	for (i = 0; i < 5; i++)
	{
		if (command[i] == create[i])
		{
			createCount++;
		}
	} 
	for (i = 0; i < 7; i++)
	{
		if (command[i] == execute[i])
		{
			exCount++;
		}
	}
	for (i = 0; i < 6; i++)
	{
		if (command[i] == delete[i])
		{
			delCount++;
		}
	}
	if (typeCount == 4)
	{
		interrupt(0x21,0,format,0,0);
		interrupt(0x21,3,fileName,buffer,0);
		interrupt(0x21,0,buffer,0,0);
		interrupt(0x21,0,format,0,0);
	}
	else if (exCount == 7)
	{
		interrupt(0x21,0,format,0,0);
		interrupt(0x21,0,format,0,0);
		interrupt(0x21,0,"Executing: ",0,0);
		interrupt(0x21,0,fileName,0,0);
		interrupt(0x21,0,format,0,0);
		interrupt(0x21,0,format,0,0);
		interrupt(0x21,4,fileName,0x2000,0);
		
	}
	else if (delCount == 6)
	{
		interrupt(0x21,7,fileName,0,0);
	}
	else if (copyCount == 4)
	{
		interrupt(0x21,3,fileName,buffer,0);
		while (buffer[fileLength] != 0x0)
		{
			fileLength++;
		}
		sectors = fileLength/512;
		interrupt(0x21,8,fileName2,buffer,sectors+1);
	}
	else if (createCount == 5)
	{
		interrupt(0x21,0,"Text file created",0,0);
		interrupt(0x21,0,format,0,0);
		interrupt(0x21,0,format,0,0);
		while(1)
		{
			for (i = 0; i < 80; i++)
			{
				input[i] = 0x0;
			}
			interrupt(0x21,1,input,0,0);
			if (input[2] == 0x0)
			{
				interrupt(0x21,0,"Text File ended",0,0);
				interrupt(0x21,0,format,0,0);
				interrupt(0x21,0,format,0,0);
				break;
			}
			else
			{
			
			for (i = 0; i < 80; i++)
			{
				if (input[i] == 0x0 || input[i] == '\0' || input[i] == '\r' || input[i] == '\n' || input[i] == '\t')
					input[i] = 0x20;
				buffer[i+80*index] = input[i];	
			}
			sector++;
			index++;
			}
		}
		interrupt(0x21,8,fileName,buffer,sector);
		
	}
	else if (dirCount == 3)
	{	
		for (i = 0; i < 10; i++)
			dirName[i] = 0x0;
		interrupt(0x21,2,buffer,2,0);
		for (i = 0; i < 16; i++)
		{
			internalCount = 0;
			if (buffer[32*i] != 0x0)
			{
				internalCount++;
				for (j = 0; j < 6; j++)
				{ 
					dirName[j] = buffer[32*i+j];					count++;
				}
				interrupt(0x21,0,format,0,0);
			}
		if (internalCount == 1)
		interrupt(0x21,0,dirName,0,0);
		}
		interrupt(0x21,0,format,0,0);
		interrupt(0x21,0,format,0,0);
	}
	else if (killCount == 4)
	{
		killer = fileName[0] - '0';
		interrupt(0x21,9,killer,0,0);
		interrupt(0x21,0,format,0,0);
		interrupt(0x21,0,"Process terminated!",0,0);
		interrupt(0x21,0,format,0,0);
		interrupt(0x21,0,format,0,0);
	}
	else
		interrupt(0x21,0, "Invalid Command\n\r",0,0);
	}

}



