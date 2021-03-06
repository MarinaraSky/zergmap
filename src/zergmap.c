#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "zerg/zergStructs.h"
#include "zerg/zergProtos.h"
#include "graph/Graph.h"
#include "dijkstra/Dijkstra.h"


int
main(
    int argc,
    char *argv[])
{
    if (argc > 1)
    {
        double          health = .1;
        char           *pHealth = NULL;
        signed int      changeLimit = 0;
        char           *pChangeLimit = NULL;

        for (int i = 1; i < argc; i++)
        {
            /* Parsing for Health switch */
            if (i < argc - 1 && strcmp(argv[i], "-h") == 0)
            {
                if (i + 1 < argc)
                {
                    health = strtod(argv[i + 1], &pHealth) / 100;
                    /* Incorrect usage of -h */
                    if (health < 0 || *pHealth != '\0')
                    {
                        printf
                            ("Usage: ./zergmap <filename> [-n <limit>] [-h <threshold>]\n");
                        return 1;
                    }
                    i++;
                }
            }
            /* Parsing for Number to delete */
            else if (i < argc - 1 && strcmp(argv[i], "-n") == 0)
            {
                if (i + 1 < argc)
                {
                    changeLimit = strtol(argv[i + 1], &pChangeLimit, 10);
                    if (changeLimit < 0 || *pChangeLimit != '\0')
                    {
                        /* Incorrect usage of -n */
                        printf
                            ("Usage: ./zergmap <filename> [-n <limit>] [-h <threshold>]\n");
                        return 1;
                    }
                    i++;
                }
            }
            /* If switched was used with no suitable params */
            else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "-h") == 0)
            {
                printf
                    ("Usage: ./zergmap <filename> [-n <limit>] [-h <threshold>]\n");
                return 1;
            }
        }
        FILE           *psychicCapture;
        int             zergCount = 0;
        ZergUnit      **unitList = calloc(sizeof(ZergUnit *), 1);

        if (!unitList)
        {
            fprintf(stderr, "Unable to allocate memory\n");
            return 1;
        }
        for (int i = 1; i < argc; i++)
        {
            strtod(argv[i], &pHealth);
            /* Skip argv arguments that arent filenames */
            if (*pHealth == '\0' || argv[i][0] == '-')
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
            /* Mostly unchanged file IO from codec */
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
                parseCapture(psychicCapture, unitList, &zergCount);
                unitList =
                    realloc(unitList, sizeof(ZergUnit *) * (zergCount + 1));
            }
            fclose(psychicCapture);
        }
        /* Prints Health of all zergs that had GPS packets */
        printf("----Zerg Health----\n");
        for (int j = 0; j < zergCount; j++)
        {
            if (unitList[j]->status &&
                (double) unitList[j]->status->currHitPoints /
                unitList[j]->status->maxHitPoints < health)
            {
                printf("Zerg ID: %hu\tHealth: %1.2lf%%\n", unitList[j]->id,
                       (double) unitList[j]->status->currHitPoints /
                       unitList[j]->status->maxHitPoints * 100);
            }
            else if (!unitList[j]->status)
            {
                printf("Zerg ID: %hu\tHealth: Not Found\n", unitList[j]->id);
            }
        }
        int             tmpCount = zergCount;

        puts("");
        Graph          *zergGraph = Graph_create();

        for (int i = 0; i < zergCount; i++)
        {
            char           *name = malloc(8);

            if (!name)
            {
                fprintf(stderr, "Unable to allocate memory.\n");
                return 1;
            }
            sprintf(name, "%hu", unitList[i]->id);
            Graph_addNode(zergGraph, name);
            for (int j = 0; j < zergCount; j++)
            {
                if (unitList[i]->id != unitList[j]->id)
                {
                    char           *next = calloc(8, 1);

                    sprintf(next, "%hu", unitList[j]->id);
                    Graph_addNode(zergGraph, next);
                    /* Adds edge if they are close enough */
                    if (zergUnit_distance(unitList[i], unitList[j]) < 15)
                    {
                        Graph_addEdge(zergGraph, name, next, 1);
                    }
                    free(next);
                }
            }
            free(name);
        }
        /* Checks for duplicates and halts if found */
        for (int j = 0; j < zergCount; j++)
        {
            if (unitList[j]->dupe == true)
            {
                fprintf(stderr, "Duplicates found.\n");
                for (int k = 0; k < zergCount; k++)
                {
                    if (unitList[k]->loc)
                    {
                        free(unitList[k]->loc);
                    }
                    if (unitList[k]->status)
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
        char          **results =
            Zerg_twoPaths(zergGraph, unitList, &zergCount, changeLimit);
        if (!results)
        {
            printf("TOO MANY CHANGES REQUIRED.\n");

        }
        else if (strcmp(results[0], "NONE") == 0)
        {
            printf("ALL ZERG ARE IN POSITION\n");
            free(results[0]);
            for (int j = 0; j < tmpCount; j++)
            {
                if (unitList[j]->loc)
                {
                    free(unitList[j]->loc);
                }
                if (unitList[j]->status)
                {
                    free(unitList[j]->status);
                }
                free(unitList[j]);
            }
        }
        else
        {
            printf("Network Alterations:\n");
            for (int i = 0; i < zergCount; i++)
            {
                printf("Remove Zerg #%s\n", results[i]);
                free(results[i]);
            }
            for (int j = 0; j < tmpCount - zergCount; j++)
            {
                if (unitList[j]->loc)
                {
                    free(unitList[j]->loc);
                }
                if (unitList[j]->status)
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
    puts("");
    return 0;
}
