#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "zerg/zergStructs.h"
#include "zerg/zergProtos.h"
#include "graph/Graph.h"
#include "dijkstra/Dijkstra.h"

char ** Zerg_twoPaths(Graph *zergGraph, ZergUnit **unitList, int *zergCount);	
void
deleteRoute(ZergUnit **route, char *node, int count);

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
				//unitList[zergCount] = create_unit();
				parseCapture(psychicCapture, unitList, &zergCount);
				/*
				for(int j = 0; j < zergCount; j++)
				{
					if(unitList[zergCount]->id == unitList[j]->id)
					{
						if(!unitList[zergCount]->status && unitList[j]->status)
						{
							unitList[zergCount]->status = unitList[j]->status;
						}
						else if(!unitList[zergCount]->loc && unitList[j]->loc)
						{
							unitList[zergCount]->loc = unitList[j]->loc;
						}
						else if(unitList[zergCount]->loc == unitList[j]->loc)
						{
							fprintf(stderr, "Multiple GPS from same id.\n");
						}
					}
				}
				*/
				//zergCount++;
				unitList = realloc(unitList, sizeof(ZergUnit*) * (zergCount + 1));
			}
        	fclose(psychicCapture);
		}
		Graph *zergGraph = Graph_create();
		for(int i = 0; i < zergCount; i++)
		{
			char *name = malloc(8);
			sprintf(name, "%hu", unitList[i]->id);
			Graph_addNode(zergGraph, name);
			for(int j = 0; j < zergCount; j++)
			{
				if(unitList[i]->id != unitList[j]->id)
				{
					char *next = calloc(8, 1);
					sprintf(next, "%hu", unitList[j]->id);
					Graph_addNode(zergGraph, next);
					if(unitList[i]->loc == unitList[j]->loc)
					{
						continue;
					}
					if(zergUnit_distance(unitList[i], unitList[j]) < 1.143)
					{
						Graph_addEdge(zergGraph, name, next, 1);
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
		for(int j = 0; j < zergCount; j++)
		{
			if(unitList[j]->dupe == true)
			{
				printf("Duplicates found.\n");
				for(int k = 0; k < zergCount; k++)
				{
					if(unitList[k]->loc)
					{
						free(unitList[k]->loc);
					}
					if(unitList[k]->status)
					{
						free(unitList[k]->status);
					}
					free(unitList[k]);
				}
				free(unitList);
				Graph_disassemble(zergGraph);
				return 1;
			}
		}
		int tmpCount = zergCount;
		char **results = Zerg_twoPaths(zergGraph, unitList, &zergCount);	
		if(!results)
		{
			printf("TOO MANY CHANGES REQUIRED.\n");

		}
		else if(strcmp(results[0], "NONE") == 0)
		{
			printf("ALL ZERG ARE IN POSITION\n");
			free(results[0]);
			printf("----Zerg Health----\n");
			for(int j = 0; j < tmpCount; j++)
			{
				if(unitList[j]->status && (double) unitList[j]->status->currHitPoints / unitList[j]->status->maxHitPoints < .1)
				{
					printf("Zerg ID: %hu\tHealth: %1.0lf%%\n", unitList[j]->id, (double) unitList[j]->status->currHitPoints / unitList[j]->status->maxHitPoints * 100);
				}
				else if(!unitList[j]->status)
				{
					printf("Zerg ID: %hu\tHealth: Not Found\n", unitList[j]->id);
				}
				if(unitList[j]->loc)
				{
					free(unitList[j]->loc);
				}
				if(unitList[j]->status)
				{
					free(unitList[j]->status);
				}
				free(unitList[j]);
			}
		}
		else
		{
			printf("Network Alterations:\n");
			for(int i = 0; i < zergCount; i++)
			{
				printf("Remove Zerg #%s\n", results[i]);
				free(results[i]);
			}
			printf("----Zerg Health----\n");
			for(int j = 0; j < tmpCount - zergCount; j++)
			{
				if(unitList[j]->status && (double) unitList[j]->status->currHitPoints / unitList[j]->status->maxHitPoints < .1)
				{
					printf("Zerg ID: %hu\tHealth: %1.0lf%%\n", unitList[j]->id, (double) unitList[j]->status->currHitPoints / unitList[j]->status->maxHitPoints * 100);
				}
				else if(!unitList[j]->status)
				{
					printf("Zerg ID: %hu\tHealth: Not Found\n", unitList[j]->id);
				}
				if(unitList[j]->loc)
				{
					free(unitList[j]->loc);
				}
				if(unitList[j]->status)
				{
					free(unitList[j]->status);
				}
				free(unitList[j]);
			}
		}
		free(unitList);
		free(results);
		Graph_disassemble(zergGraph);
    }
    return 0;
}

char **
Zerg_twoPaths(Graph *zergGraph, ZergUnit **unitList, int *zergCount)	
{
	char **deletions = calloc(1, sizeof(*deletions) * *zergCount);
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
				if(adjacent && zergUnit_distance(unitList[i], unitList[j]) < 1.143)
				{
					deletions[delTrack] = calloc(8, 1);
					strcpy(deletions[delTrack], next);
					delTrack++;
					deleteRoute(unitList, next, *zergCount);
					Graph_deleteNode(zergGraph, next);
					*zergCount -= 1;
				}
				for(ssize_t y = 0; y < hops -1; y++)
				{
					if(y != hops)
					{
						if(route[y] && route[y+1])
						{
							Graph_deleteEdge(zergGraph, route[y], route[y+1]);	
						}
					}
				}
				ssize_t newHops = Dijkstra_path(zergGraph, name, next, &newRoute); 
				if(newHops == 1 && !adjacent)
				{
					deletions[delTrack] = calloc(8, 1);
					strcpy(deletions[delTrack], newRoute[0]);
					delTrack++;
					deleteRoute(unitList, newRoute[0], *zergCount);
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
				free(name);
				free(next);
				free(route);
				free(newRoute);
			}
		}
	}
	*zergCount = delTrack;
	if(delTrack > tmpCount)
	{
		for(int i = 0; i < tmpCount * 2; i++)
		{
			free(deletions[i]);
		}
		for(int i = 0; i < *zergCount - 1; i++)
		{
			if(unitList[i]->loc)
			{
				free(unitList[i]->loc);
			}
			if(unitList[i]->status)
			{
				free(unitList[i]->status);
			}
			if(unitList[i])
			{
				free(unitList[i]);
			}
		}
		free(deletions);
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

void
deleteRoute(ZergUnit **route, char *node, int count)
{
	for(int i = 0; i < count; i++)
	{
		char *cmp = malloc(8);
		sprintf(cmp, "%d", route[i]->id);
		if(strcmp(cmp, node) == 0)
		{
			if(route[i]->loc)
			{
				free(route[i]->loc);
			}
			if(route[i]->status)
			{
				free(route[i]->status);
			}
			free(route[i]);
			for(int j = i + 1; j < count; j++)
			{
				route[i] = route[j];	
			}
		}
		free(cmp);
	}
}
