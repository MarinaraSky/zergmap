#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zergStructs.h"
#include "zergProtos.h"
#include "graph/Graph.h"

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
		for(int i = 1; i < argc; i++)
		{
			psychicCapture = fopen(argv[i], "rb");
			printf("FILE: %s\n", argv[i]);
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
				unitList[zergCount] = create_unit();
				parseCapture(psychicCapture, unitList[zergCount]);
				zergCount++;
				unitList = realloc(unitList, sizeof(ZergUnit*) * (zergCount + 1));
			}
		}
		Graph *zergGraph = Graph_create();
		int tracker = 0;
		for(int i = 0; i < zergCount; i++)
		{
			tracker = i;
			char *name = malloc(8);
			sprintf(name, "%hu", unitList[i]->id);
			Graph_addNode(zergGraph, name);
			for(int j = 0; j < zergCount; j++)
			{
				if(tracker != j)
				{
					char *next = malloc(8);
					sprintf(next, "%hu", unitList[j]->id);
					Graph_addNode(zergGraph, next);
					printf("Distance between %s and %s: %lf\n", name, next, zergUnit_distance(unitList[i], unitList[j]));
					if(zergUnit_distance(unitList[i], unitList[j]) < 1.143)
					{
						printf("%s, %s to close.\n", name, next);
					}
					if(zergUnit_distance(unitList[i], unitList[j]) < 15)
					{
						Graph_addEdge(zergGraph, name, next, 1);
					}
					free(next);
				}
			}
			free(name);
		}
		Graph_print(zergGraph);
        fclose(psychicCapture);
    }
    return 0;
	}
