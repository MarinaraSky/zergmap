#ifndef zergProtos_H_
#define zergProtos_H_

#include "zergStructs.h"
#include "../graph/Graph.h"
#include <stdio.h>
/**************ENCODE FUNCTIONS*******************/
/* Read Zerg Header and send to parsing function */
void            readZerg(
    FILE * source,
    FILE * dest);

/* Collects information about packet */
void            checkEntry(
    char string[16],
    unsigned int input,
    zergPacket * packet);

/* Chooses the type of packet */
void            pickPacketType(
    FILE * source,
    FILE * dest,
    zergPacket * packet);

/* Writes the packet to a file if its a message */
void            writeMessage(
    FILE * source,
    FILE * dest);

/* Writes the packet to a file if its a status */
void            writeStatus(
    FILE * source,
    FILE * dest);

/* Writes the packet to a file if its a command */
void            writeCommand(
    FILE * source,
    FILE * dest);

/* Writes the packet to a file if its a GPS */
void            writeGPS(
    FILE * source,
    FILE * dest);

/* Writes the Pcap Header to file */
void            writePcapHeader(
    FILE * dest);

/* Writes the Pcap Packet to file */
void            writePcapPacket(
    FILE * dest,
    int zergLength);

/* Writes the Ether Header to file */
void            writeEtherHeader(
    FILE * dest);

/* Writes the Ipv4 Header to file */
void            writeIpv4Header(
    FILE * dest,
    int zergLength);

/* Writes the UDP Header to file */
void            writeUdpHeader(
    FILE * dest,
    int zergLength);

/* Writes the Zerg Header to file */
void            writeZergHeader(
    FILE * dest,
    zergPacket * packet);

/* Rotates a 3 byte integer endianess */
int             rotate3ByteInt(
    int swap);

/* Reverses the rotation */
int             rotateBack(
    int swap);

/* Used to exit if anything goes wrong with reading the file */
void            fileCorruption(
    void);

/* Used to check if there are invalid inputs in Zerg Header */
int             validateHeader(
    zergPacket * packet);

/***************DECODE FUNCTIONS*****************/
ZergUnit       *create_unit(
    void);

/* Used as a hub to call the other read functions */
void            parseCapture(
    FILE * psychicCapture,
    ZergUnit ** unit,
    int *zergCount);

/* Reads the Pcap File header, contains a way to check  */
/* validity for both big endian and little endian       */
void            readPcapHeader(
    FILE * psychicCapture);

/* Reads Pcap packet header */
void            readPcapPacket(
    FILE * psychicCapture);

/* Reads Ethernet Packet Header */
void            readEthernetPacket(
    FILE * psychicCapture);

/* Reads Ipv4 Packet Header */
void            readIpv4Packet(
    FILE * psychicCapture,
    unsigned int *ipTotalLength);

/* Reads Ipv6 Packet Header */
void            readIpv6Packet(
    FILE * psychicCaputre,
    unsigned int *ipTotalLength);

/* Reads UPD Packet Header */
void            readUdpPacket(
    FILE * psychicCapture,
    unsigned int *udpTotalLength);

/* Reads Zerg Packet Header */
void            readZergPacket(
    FILE * psychicCapture,
    unsigned int *udpTotalLength,
    ZergUnit ** unit,
    int *zergCount);

/* Prints Message to screen */
void            readMessage(
    FILE * psychicCapture,
    unsigned int payloadLength);

/* Reads Status packet and prints to screen */
void            readStatus(
    FILE * psychicCapture,
    unsigned int payloadLength,
    ZergUnit * unit);

/* Readas Command packet and prints to screen */
void            readCommand(
    FILE * psychicCapture);

/* Reads GPS Packets and prints to screen */
void            readGPS(
    FILE * psychicCapture,
    ZergUnit * unit);

/* Takes a char and bitwise manipulates it into an Int */
void            hexToInt(
    unsigned int *myInt,
    unsigned char hex);

/* Takes a char and bitwise manipulates it into a short */
void            hexToShort(
    unsigned short *myShort,
    unsigned char hex);

/* Takes a char and bitwise manipulates it into a double */
void            hexToDouble(
    unsigned long long *myLong,
    unsigned char hex);

/* Converts a decimal to degrees minutes seconds for gps */
void            decimalDegreesToDMS(
    double coordinate);

/**
 * @brief Calculates the difference in location between two
 * zerg units.
 * @param z1 From Zerg
 * @param z2 To Zerg
 * @return Distance as Double
 */
double          zergUnit_distance(
    ZergUnit * z1,
    ZergUnit * z2);

/**
 * @brief Prints id and GPS information for ZergUnit
 */
void            print_zergUnit(
    ZergUnit * z);

/**
 * @brief My attempt to find disjointed node paths between two nodes
 * @param zergGraph Graph of ZergUnits to check
 * @param unitList Array of ZergUnit pointers
 * @param zergCount Pointer to number of ZergUnits in unitList
 * @param changeLimit Used to set how many Zergs can be deleted
 * @return List of node id's that were deleted
 */
char          **Zerg_twoPaths(
    Graph * zergGraph,
    ZergUnit ** unitList,
    int *zergCount,
    int changeLimit);

/**
 * @brief Used to delete and shift zergUnit list after deletion
 * @param route zergUnit list that had been traversed
 * @param node Zerg id as a string to be removed
 * @param count Pointer to count to keep track of number of zergUnits
 */
void            deleteRoute(
    ZergUnit ** route,
    char *node,
    int *count);
#endif
