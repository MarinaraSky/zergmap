#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zergStructs.h"
#include "zergProtos.h"

int
main(
    int argc,
    char *argv[])
{
    if (argc > 1)
    {
        FILE           *psychicCapture;
		
		int zergCount = 0;
		ZergUnit **unitList = calloc(sizeof(ZergUnit*), 1);
		unitList[zergCount] = create_unit();
		for(int i = 1; i < argc; i++)
		{
			psychicCapture = fopen(argv[i], "rb");
			if (psychicCapture == NULL)
			{
				printf("Cannot open %s\n", argv[1]);
				exit(1);
			}
			readPcapHeader(psychicCapture);
			while (!feof(psychicCapture))
			{
				if (getc(psychicCapture) == EOF)
				{
					break;
				}
				else
				{
					fseek(psychicCapture, -1, SEEK_CUR);
				}
				parseCapture(psychicCapture, unitList[zergCount]);
				print_zergUnit(unitList[zergCount]);
				zergCount++;
				unitList = realloc(unitList, sizeof(ZergUnit*) * (zergCount + 1));
				unitList[zergCount] = create_unit();
			}
		}
		printf("distance between: %lf\n", zergUnit_distance(unitList[0], unitList[1] ) * 1000);
        fclose(psychicCapture);
    }
    return 0;
}
