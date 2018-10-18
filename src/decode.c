#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zerg/zergStructs.h"
#include "zerg/zergProtos.h"
#include "graph/Graph.h"
#include "dijkstra/Dijkstra.h"

char **
Zerg_twoPaths(Graph *zergGraph, ZergUnit **unitList, int *zergCount);	

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
				if(zergCount > 0 && unitList[zergCount]->id == unitList[zergCount - 1]->id)
				{
					printf("Duplicate GPS Senders. Quitting\n");
					return 1;
				}
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
        fclose(psychicCapture);
		char **results = Zerg_twoPaths(zergGraph, unitList, &zergCount);	
		if(!results)
		{
			printf("TOO MANY CHANGES REQUIRED.\n");
		}
		else if(strcmp(results[0], "NONE") == 0)
		{
			printf("ALL ZERG ARE IN POSITION\n");
		}
		else
		{
			printf("Network Alterations:\n");
			for(int i = 0; i < zergCount; i++)
			{
				printf("Remove Zerg #%s\n", results[i]);
			}
		}
    }
    return 0;
}

char **
Zerg_twoPaths(Graph *zergGraph, ZergUnit **unitList, int *zergCount)	
{
	char **deletions = malloc(sizeof(*deletions));
	int delTrack = 0;
	int tmpCount = *zergCount / 2;
	for(int i = 0; i < *zergCount; i++)
	{
		for(int j = 0; j < *zergCount; j++)
		{
			if(i != j)
			{
				char **route;
				char **newRoute;
				char *name = malloc(8);
				char *next = malloc(8);
				sprintf(name, "%hu", unitList[i]->id);
				sprintf(next, "%hu", unitList[j]->id);
				ssize_t hops = Dijkstra_path(zergGraph, name, next, &route); 
				bool adjacent = Graph_isAdjacent(zergGraph, name, next);
				for(ssize_t y = 0; y < hops; y++)
				{
					if(y != hops - 1)
					{
						Graph_deleteEdge(zergGraph, route[y], route[y+1]);	
					}
				}
				ssize_t newHops = Dijkstra_path(zergGraph, name, next, &newRoute); 
				if(newHops == 1 && !adjacent)
				{
					deletions[delTrack] = malloc(8);
					strcpy(deletions[delTrack], newRoute[0]);
					delTrack++;
					deletions = realloc(deletions, sizeof(*deletions) * delTrack + 1);
					for(int z = 0; z < *zergCount; z++)
					{
						char *cmp = malloc(8);
						sprintf(cmp, "%d", unitList[z]->id);
						if(strcmp(cmp, newRoute[0]) == 0)
						{
							unitList[z] = unitList[i];
						}
					}
					Graph_deleteNode(zergGraph, newRoute[0]);
					*zergCount -= 1;
				}	
				for(ssize_t y = 0; y < hops; y++)
				{
					if(y != hops - 1)
					{
						Graph_addEdge(zergGraph, route[y], route[y+1], 1);	
					}
				}
				
			}
		}
	}
	*zergCount = delTrack;
	if(delTrack > tmpCount)
	{
		return NULL;
	}
	if(delTrack > 0)
	{
		return deletions;
	}
	else
	{
		deletions[0] = malloc(8);
		strcpy(deletions[0], "NONE");
		return deletions;
	}
}
