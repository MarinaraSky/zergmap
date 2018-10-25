#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "zerg/zergStructs.h"
#include "zerg/zergProtos.h"
#include "graph/Graph.h"
#include "dijkstra/Dijkstra.h"

char ** Zerg_twoPaths(Graph *zergGraph, ZergUnit **unitList, int *zergCount, int changeLimit);	
void
deleteRoute(ZergUnit **route, char *node, int count);

int
main(
    int argc,
    char *argv[])
{
    if (argc > 1)
    {
		double health = .1;
		char *pHealth = NULL;
		signed int changeLimit = 0;
		char *pChangeLimit = NULL;
		for(int i = 1; i < argc; i++)
		{
			if(i < argc - 1 && strcmp(argv[i], "-h") == 0)
			{
				if(i + 1 < argc)
				{
					health = strtod(argv[i + 1], &pHealth) / 100;
					if(*pHealth != '\0')
					{
						printf("Usage: ./zergmap <filename> [-n <limit>] [-h <threshold>]\n");
						return 1;
					}
					i++;
				}
			}
			else if(i < argc - 1 && strcmp(argv[i], "-n") == 0)
			{
				if(i + 1 < argc)
				{
					changeLimit = strtol(argv[i + 1], &pChangeLimit, 10);
					if(changeLimit < 0 || *pChangeLimit != '\0')
					{
						printf("Usage: ./zergmap <filename> [-n <limit>] [-h <threshold>]\n");
						return 1;
					}
					i++;
				}
			}
			else if(strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "-h") == 0)
			{
				printf("Usage: ./zergmap <filename> [-n <limit>] [-h <threshold>]\n");
				return 1;
			}
		}
        FILE           *psychicCapture;
		int zergCount = 0;
		ZergUnit **unitList = calloc(sizeof(ZergUnit*), 1);
		for(int i = 1; i < argc; i++)
		{
			strtod(argv[i], &pHealth);
			if(*pHealth == '\0' || argv[i][0] == '-')
			{
				i++;
				continue;
			}
			psychicCapture = fopen(argv[i], "rb");
			if (psychicCapture == NULL)
			{
				printf("Cannot open %s\n", argv[i]);
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
				{ fseek(psychicCapture, -1, SEEK_CUR);
				}
				parseCapture(psychicCapture, unitList, &zergCount);
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
				fprintf(stderr, "Duplicates found.\n");
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
		printf("----Zerg Health----\n");
		for(int j = 0; j < zergCount; j++)
		{
			if(unitList[j]->status && (double) unitList[j]->status->currHitPoints / unitList[j]->status->maxHitPoints < health)
			{
				printf("Zerg ID: %hu\tHealth: %1.2lf%%\n", unitList[j]->id, 
						(double) unitList[j]->status->currHitPoints / unitList[j]->status->maxHitPoints * 100);
			}
			else if(!unitList[j]->status)
			{
				printf("Zerg ID: %hu\tHealth: Not Found\n", unitList[j]->id);
			}
		}
		int tmpCount = zergCount;
		char **results = Zerg_twoPaths(zergGraph, unitList, &zergCount, changeLimit);	
		if(!results)
		{
			printf("TOO MANY CHANGES REQUIRED.\n");

		}
		else if(strcmp(results[0], "NONE") == 0)
		{
			printf("ALL ZERG ARE IN POSITION\n");
			free(results[0]);
			for(int j = 0; j < tmpCount; j++)
			{
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
			for(int j = 0; j < tmpCount - zergCount; j++)
			{
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
Zerg_twoPaths(Graph *zergGraph, ZergUnit **unitList, int *zergCount, int changeLimit)	
{
	char **deletions = calloc(1, sizeof(*deletions) * *zergCount);
	int delTrack = 0;
	int tmpCount = 0;
	int origCount = *zergCount;
	if(changeLimit == 0)
	{
		tmpCount = *zergCount / 2;
	}
	else
	{
		tmpCount = changeLimit;
	}
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
					
				char **split = calloc(1, sizeof(*split) * 10);
				int splitCount = 0;
				for(int z = 1; z < hops - 1; z++)
				{
					for(int a = 1; a < newHops - 1; a++)
					{
						if(strcmp(route[z], newRoute[a]) == 0)
						{
							split[splitCount++] = strdup(newRoute[a]);
						}
					}		
				}
				if((newHops == 1 && !adjacent) || (!adjacent && splitCount == 1))
				{
					char **neighbors = NULL;
					if(Graph_getNeighbors(zergGraph, name, &neighbors) < Graph_getNeighbors(zergGraph, next, &neighbors))
					{
						deletions[delTrack] = calloc(8, 1);
						strcpy(deletions[delTrack], name);
						delTrack++;
						deleteRoute(unitList, name, *zergCount);
						Graph_deleteNode(zergGraph, name);
						*zergCount -= 1;
					}
					else
					{
						deletions[delTrack] = calloc(8, 1);
						strcpy(deletions[delTrack], next);
						delTrack++;
						deleteRoute(unitList, next, *zergCount);
						Graph_deleteNode(zergGraph, next);
						*zergCount -= 1;
					}	
				}
				for(int z = 0; z < splitCount; z++)
				{
					free(split[z]);
				}
				free(split);
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
		for(int i = 0; i < origCount; i++)
		{
			free(deletions[i]);
		}
		for(int i = 0; i < origCount - *zergCount; i++)
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
