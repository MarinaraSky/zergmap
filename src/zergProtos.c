#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <byteswap.h>
#include "zergProtos.h"
#include "zergStructs.h"

int             zergPayloadSize = 0;
int             fscanNum = 0;

void
readZerg(
    FILE * source,
    FILE * dest)
{
    zergPacket     *packet = calloc(sizeof(zergPacket), 1);

    if (packet == NULL)
    {
        printf("Unable to allocate memory. Quitting.\n");
        exit(1);
    }
    int             readHeader = 0;
    char            string[16] = "";
    unsigned int    input = 0;

    while (readHeader < 7)
    {
        fscanf(source, "%s %u", string, &input);
        checkEntry(string, input, packet);
        readHeader++;
    }
    if (validateHeader(packet))
    {
        pickPacketType(source, dest, packet);
        free(packet);
    }
}

void
checkEntry(
    char string[16],
    unsigned int input,
    zergPacket * packet)
{
    if (strcmp("Version:", string) == 0)
    {
        packet->version = input;
    }
    else if (strcmp("Type:", string) == 0)
    {
        packet->type = input;
    }
    else if (strcmp("Size:", string) == 0)
    {
        packet->totalLength = input;
        zergPayloadSize = input - 13;
    }
    else if (strcmp("From:", string) == 0)
    {
        packet->sourceId = input;
    }
    else if (strcmp("To:", string) == 0)
    {
        packet->destinationId = input;
    }
    else if (strcmp("Sequence:", string) == 0)
    {
        packet->sequenceId = input;
    }
}

void
pickPacketType(
    FILE * source,
    FILE * dest,
    zergPacket * packet)
{
    if (packet->type == 2)
    {
        writePcapPacket(dest, packet->totalLength + 4);
    }
    else
    {
        writePcapPacket(dest, packet->totalLength);
    }
    writeEtherHeader(dest);
    writeIpv4Header(dest, packet->totalLength);
    writeUdpHeader(dest, packet->totalLength);
    writeZergHeader(dest, packet);
    switch (packet->type)
    {
    case (0):
    {
        writeMessage(source, dest);
        break;
    }
    case (1):
    {
        writeStatus(source, dest);
        break;
    }
    case (2):
    {
        writeCommand(source, dest);
        break;
    }
    case (3):
    {
        writeGPS(source, dest);
        break;
    }
    default:
    {
        printf("Not Found\n");
    }
    }
}

void
writeMessage(
    FILE * source,
    FILE * dest)
{
    char            grab = 0;

    fseek(source, 1, SEEK_CUR);
    for (int i = 0; i <= zergPayloadSize; i++)
    {
        grab = fgetc(source);
        if (grab == EOF)
        {
            fclose(source);
            fclose(dest);
            fileCorruption();
        }
        fputc(grab, dest);
    }
}

void
writeStatus(
    FILE * source,
    FILE * dest)
{
    char            string[16] = "";
    int             health = 0;
    int             maxHealth = 0;
    int             type = 0;
    int             armor = 0;
    payload        *status = calloc(sizeof(payload), 1);

    if (status == NULL)
    {
        printf("Cannot allocate memory\n");
        exit(1);
    }

    for (int i = 0; i < 9; i++)
    {
        fscanNum = fscanf(source, "%s", string);
        if (fscanNum != 1)
        {
            free(status);
            fclose(source);
            fclose(dest);
            fileCorruption();
        }
        if (strcmp("Type:", string) == 0)
        {
            fscanNum = fscanf(source, "%d", &type);
            if (fscanNum != 1)
            {
                free(status);
                fclose(source);
                fclose(dest);
                fileCorruption();
            }
            status->type = type & 0xff;
        }
        else if (strcmp("Speed:", string) == 0)
        {
            fscanNum = fscanf(source, "%s", string);
            if (fscanNum != 1)
            {
                free(status);
                fclose(source);
                fclose(dest);
                fileCorruption();
            }
            status->sSpeed.fSpeed = strtof(string, NULL);
            status->sSpeed.iSpeed = htonl(status->sSpeed.iSpeed);
        }
        if (strcmp("Health:", string) == 0)
        {
            fscanNum = fscanf(source, "%d/%d", &health, &maxHealth);
            if (fscanNum != 2)
            {
                free(status);
                fclose(source);
                fclose(dest);
                fileCorruption();
            }
            status->currHitPoints |= health;
            status->maxHitPoints |= maxHealth;
            status->currHitPoints = rotate3ByteInt(status->currHitPoints);
            status->maxHitPoints = rotate3ByteInt(status->maxHitPoints);
        }
        else if (strcmp("Armor:", string) == 0)
        {
            fscanNum = fscanf(source, "%d", &armor);
            if (fscanNum != 1)
            {
                free(status);
                fclose(source);
                fclose(dest);
                fileCorruption();
            }
            status->armor = armor & 0xff;
            fwrite(status, sizeof(int) * 3, 1, dest);
        }
        else if (strcmp("Name:", string) == 0)
        {
            char            grab = 0;

            fseek(source, 1, SEEK_CUR);
            for (int i = 0; i <= zergPayloadSize - 12; i++)
            {
                grab = fgetc(source);
                if (grab == EOF)
                {
                    free(status);
                    fclose(source);
                    fclose(dest);
                    fileCorruption();
                }
                fputc(grab, dest);
            }
            free(status);
            return;
        }
    }
}

void
writeCommand(
    FILE * source,
    FILE * dest)
{
    char            string[16] = "";
    char            garbage[16] = "";
    short int       input = 0;
    int             intInput = 0;
    cPayload       *command = calloc(sizeof(cPayload), 1);

    if (command == NULL)
    {
        printf("Cannot allocate memory\n");
        exit(1);
    }
    fscanNum = fscanf(source, "%s %hd %s", string, &input, garbage);
    if (fscanNum != 3)
    {
        free(command);
        fclose(source);
        fclose(dest);
        fileCorruption();
    }
    if (strcmp("Command:", string) == 0)
    {
        if (input < 8)
        {
            command->command = htons(input);
        }
        else
        {
            free(command);
            fclose(source);
            fclose(dest);
            fileCorruption();
        }
    }
    switch (htons(command->command))
    {
    case (1):
        fscanNum = fscanf(source, "%s %hd", string, &input);
        if (fscanNum != 2)
        {
            free(command);
            fclose(source);
            fclose(dest);
            fileCorruption();
        }
        command->param1 = htons(input);
        float           fInput = 0.0;

        fscanNum = fscanf(source, "%s %f", string, &fInput);
        if (fscanNum != 2)
        {
            free(command);
            fclose(source);
            fclose(dest);
            fileCorruption();
        }
        command->param2.fParam2 = fInput;
        command->param2.iParam2 = htonl(command->param2.iParam2);
        fwrite(command, sizeof(int) * 2, 1, dest);
        break;
    case (5):
        fscanNum = fscanf(source, "%s %hd", string, &input);
        if (fscanNum != 2)
        {
            free(command);
            fclose(source);
            fclose(dest);
            fileCorruption();
        }
        command->param1 = htons(input & 0x1);
        fscanNum = fscanf(source, "%s %d", string, &intInput);
        if (fscanNum != 2)
        {
            free(command);
            fclose(source);
            fclose(dest);
            fileCorruption();
        }
        command->param2.iParam2 = htonl(intInput);
        fwrite(command, sizeof(int) * 2, 1, dest);
        break;
    case (7):
        fscanNum = fscanf(source, "%s %d", string, &intInput);
        if (fscanNum != 2)
        {
            free(command);
            fclose(source);
            fclose(dest);
            fileCorruption();
        }
        command->param2.iParam2 = htonl(intInput);
        fwrite(command, sizeof(int) * 2, 1, dest);
        break;
    default:
        fwrite(command, sizeof(char) * 6, 1, dest);
        break;
    }
    free(command);
}

void
writeGPS(
    FILE * source,
    FILE * dest)
{
    char            string[16] = "";
    double          dInput = 0.0;
    float           fInput = 0.0;
    char            garbage[16] = "";
    gpsPayload     *gpsCoords = calloc(sizeof(gpsPayload), 1);

    if (gpsCoords == NULL)
    {
        printf("Cannot allocate memory\n");
        exit(1);
    }

    for (int i = 0; i < 6; i++)
    {
        fscanNum = fscanf(source, "%s", string);
        if (fscanNum != 1)
        {
            free(gpsCoords);
            fclose(source);
            fclose(dest);
            fileCorruption();
        }
        if (strcmp("Long:", string) == 0)
        {
            fscanNum = fscanf(source, "%lf", &dInput);
            if (fscanNum != 1)
            {
                free(gpsCoords);
                fclose(source);
                fclose(dest);
                fileCorruption();
            }
            gpsCoords->longitude.dLong = dInput;
            gpsCoords->longitude.iLong = bswap_64(gpsCoords->longitude.iLong);
        }
        else if (strcmp("Lat:", string) == 0)
        {
            fscanNum = fscanf(source, "%lf", &dInput);
            if (fscanNum != 1)
            {
                free(gpsCoords);
                fclose(source);
                fclose(dest);
                fileCorruption();
            }
            gpsCoords->latitude.dLat = dInput;
            gpsCoords->latitude.iLat = bswap_64(gpsCoords->latitude.iLat);
        }
        else if (strcmp("Alt:", string) == 0)
        {
            fscanNum = fscanf(source, "%f %s", &fInput, garbage);
            if (fscanNum != 2)
            {
                free(gpsCoords);
                fclose(source);
                fclose(dest);
                fileCorruption();
            }
            gpsCoords->altitude.fAltitude = fInput;
            gpsCoords->altitude.iAltitude =
                htonl(gpsCoords->altitude.iAltitude);
        }
        else if (strcmp("Bearing:", string) == 0)
        {
            fscanNum = fscanf(source, "%f %s", &fInput, garbage);
            if (fscanNum != 2)
            {
                free(gpsCoords);
                fclose(source);
                fclose(dest);
                fileCorruption();
            }
            gpsCoords->bearing.fBearing = fInput;
            gpsCoords->bearing.iBearing = htonl(gpsCoords->bearing.iBearing);
        }
        else if (strcmp("Speed:", string) == 0)
        {
            fscanNum = fscanf(source, "%f %s", &fInput, garbage);
            if (fscanNum != 2)
            {
                free(gpsCoords);
                fclose(source);
                fclose(dest);
                fileCorruption();
            }
            gpsCoords->speed.fSpeed = fInput;
            gpsCoords->speed.iSpeed = htonl(gpsCoords->speed.iSpeed);
        }
        else if (strcmp("Acc:", string) == 0)
        {
            fscanNum = fscanf(source, "%f %s", &fInput, garbage);
            if (fscanNum != 2)
            {
                free(gpsCoords);
                fclose(source);
                fclose(dest);
                fileCorruption();
            }
            gpsCoords->accuracy.fAccuracy = fInput;
            gpsCoords->accuracy.iAccuracy =
                htonl(gpsCoords->accuracy.iAccuracy);
        }
        else
        {
            printf("Something's not right here. Exiting.");
            exit(1);
        }
    }
    fwrite(gpsCoords, sizeof(int) * 8, 1, dest);
    free(gpsCoords);
}

void
writePcapHeader(
    FILE * dest)
{
    pcapFileHeader *header = calloc(sizeof(pcapFileHeader), 1);

    if (header == NULL)
    {
        printf("Cannot allocate memory\n");
        exit(1);
    }

    header->fileTypeId = 0xa1b2c3d4;
    header->majorVersion = 0x2;
    header->minorVersion = 0x4;
    header->gmtOffset = 0;
    header->accDelta = 0;
    header->maxLength = 0x10000;
    header->linkLayerType = 0x1;
    fwrite(header, sizeof(int) * 6, 1, dest);
    free(header);
}

void
writePcapPacket(
    FILE * dest,
    int zergLength)
{
    pcapPacketHeader *header = calloc(sizeof(pcapPacketHeader), 1);

    if (header == NULL)
    {
        printf("Cannot allocate memory\n");
        exit(1);
    }

    header->unixEpoch = 0x11111111;
    header->microEpoch = 0x11111111;
    header->lengthOfData = zergLength + 42;
    header->fullLength = 0x11111111;
    fwrite(header, sizeof(int) * 4, 1, dest);
    free(header);
}

void
writeEtherHeader(
    FILE * dest)
{
    ethernetHeader *header = calloc(sizeof(ethernetHeader), 1);

    if (header == NULL)
    {
        printf("Cannot allocate memory\n");
        exit(1);
    }

    header->etherType = 0x8;
    fwrite(header, sizeof(char) * 14, 1, dest);
    free(header);
}

void
writeIpv4Header(
    FILE * dest,
    int zergLength)
{
    ipv4Header     *header = calloc(sizeof(ipv4Header), 1);

    if (header == NULL)
    {
        printf("Cannot allocate memory\n");
        exit(1);
    }

    header->version = 0x4;
    header->ipHeaderLength = 0x5;
    header->ipLength = htons(zergLength + 28);
    header->protocol = 0x11;
    header->destIp = 0x22222222;
    fwrite(header, sizeof(int) * 5, 1, dest);
    free(header);
}

void
writeUdpHeader(
    FILE * dest,
    int zergLength)
{
    udpHeader      *header = calloc(sizeof(udpHeader), 1);

    if (header == NULL)
    {
        printf("Cannot allocate memory\n");
        exit(1);
    }

    header->destPort = htons(0xea7);
    header->length = htons(zergLength + 8);
    header->checksum = 0xefbe;
    fwrite(header, sizeof(int) * 2, 1, dest);
    free(header);
}

void
writeZergHeader(
    FILE * dest,
    zergPacket * packet)
{
    packet->totalLength = rotate3ByteInt(packet->totalLength);
    packet->sourceId = htons(packet->sourceId);
    packet->destinationId = htons(packet->destinationId);
    packet->sequenceId = htonl(packet->sequenceId);
    fwrite(packet, sizeof(int) * 3, 1, dest);
}

int
rotate3ByteInt(
    int swap)
{
    swap = ((swap >> 16) + (swap & 0xff00) + (swap & 0xff)) << 16;
    return swap;
}

int
rotateBack(
    int swap)
{
    swap = ((swap << 16) + (swap & 0xff00) + (swap & 0xff)) >> 16;
    return swap;
}

void
fileCorruption(
    void)
{
    printf("Unexpected data found in source document. Exiting.\n");
    exit(2);
}

int
validateHeader(
    zergPacket * packet)
{
    if (packet->version == 0 && packet->totalLength == 0 &&
        packet->sourceId == 0 && packet->destinationId == 0 &&
        packet->sequenceId == 0)
    {
        free(packet);
        return 0;
    }
    else if (packet->version == 0 || packet->totalLength == 0 ||
             packet->sourceId == 0 || packet->destinationId == 0 ||
             packet->sequenceId == 0)
    {
        fileCorruption();
    }
    return 1;
}

void
parseCapture(
    FILE * psychicCapture)
{
    unsigned int    ipTotalLength = 0;
    unsigned int    udpTotalLength = 0;

    readPcapPacket(psychicCapture);
    readEthernetPacket(psychicCapture);
    readIpv4Packet(psychicCapture, &ipTotalLength);
    readUdpPacket(psychicCapture, &udpTotalLength, ipTotalLength);
    readZergPacket(psychicCapture, &udpTotalLength);
    printf("\n");
}

void
readPcapHeader(
    FILE * psychicCapture)
{
    unsigned char   buff = 0;
    int             i = 0;
    pcapFileHeader *header = calloc(sizeof(pcapFileHeader), 1);

    while (i < 24)
    {
        buff = getc(psychicCapture);
        if (i < 4)
        {
            hexToInt(&header->fileTypeId, buff);
        }
        else if (i < 6)
        {
            hexToShort(&header->majorVersion, buff);
        }
        else if (i < 8)
        {
            hexToShort(&header->minorVersion, buff);
        }
        else if (i < 12)
        {
            hexToInt(&header->gmtOffset, buff);
        }
        else if (i < 16)
        {
            hexToInt(&header->accDelta, buff);
        }
        else if (i < 20)
        {
            hexToInt(&header->maxLength, buff);
        }
        else if (i < 24)
        {
            hexToInt(&header->linkLayerType, buff);
        }
        i++;
    }
    /* This gross if statement checks if the pcap header is valid in    */
    /* Big Endian or Little Endian                                      */
    if ((header->fileTypeId != 0xd4c3b2a1 || header->majorVersion != 0x0200 ||
         header->minorVersion != 0x0400 || header->linkLayerType != 0x1000000)
        && (header->fileTypeId != 0xa1b2c3d4 || header->majorVersion != 0x2 ||
            header->minorVersion != 0x4 || header->linkLayerType != 0x1))
    {
        printf("Pcap Header is invalid.\n");
        free(header);
        fclose(psychicCapture);
        fileCorruption();
    }
    free(header);
}

void
readPcapPacket(
    FILE * psychicCapture)
{
    unsigned char   buff = 0;
    int             i = 0;
    pcapPacketHeader *header = calloc(sizeof(pcapPacketHeader), 1);

    while (i < 16)
    {
        buff = getc(psychicCapture);
        if (i < 4)
        {
            hexToInt(&header->unixEpoch, buff);
        }
        else if (i < 8)
        {
            hexToInt(&header->microEpoch, buff);
        }
        else if (i < 12)
        {
            hexToInt(&header->lengthOfData, buff);
        }
        else if (i < 16)
        {
            hexToInt(&header->fullLength, buff);
        }
        i++;
    }
    if (header->lengthOfData == 0)
    {
        free(header);
        fclose(psychicCapture);
        printf("Pcap Packet length is invalid.\n");
        fileCorruption();
    }
    free(header);
}

void
readEthernetPacket(
    FILE * psychicCapture)
{
    unsigned char   buff = 0;
    int             i = 0;
    ethernetHeader *header = calloc(sizeof(ethernetHeader), 1);

    while (i < 15)
    {
        buff = getc(psychicCapture);
        if (i < 6)
        {
            header->sourceMac[i] = buff;
        }
        else if (i < 12)
        {
            header->destMac[i - 6] = buff;
        }
        else if (i < 14)
        {
            hexToShort(&header->etherType, buff);
        }
        i++;
    }
    if (header->etherType != 0x0800 && header->etherType != 0x86DD)
    {
        printf("EtherType is invalid.\n");
        free(header);
        fclose(psychicCapture);
        fileCorruption();
    }
    free(header);
}

void
hexToInt(
    unsigned int *myInt,
    unsigned char hex)
{
    *myInt <<= 8;
    *myInt |= hex;
}

void
hexToShort(
    unsigned short *myShort,
    unsigned char hex)
{
    *myShort <<= 8;
    *myShort |= hex;
}

void
hexToDouble(
    unsigned long long *myLong,
    unsigned char hex)
{
    *myLong <<= 8;
    *myLong |= hex;
}

void
readIpv4Packet(
    FILE * psychicCapture,
    unsigned int *ipTotalLength)
{
    unsigned char   buff = 0;
    int             i = 0;
    ipv4Header     *header = calloc(sizeof(ipv4Header), 1);

    fseek(psychicCapture, -1, SEEK_CUR);

    while (i < 20)
    {
        buff = getc(psychicCapture);
        if (i == 0)
        {
            header->ipHeaderLength = buff & 0xf;
            buff = buff >> 4;
            header->version = buff & 0xf;
            if (header->version == 0x6)
            {
                fseek(psychicCapture, -1, SEEK_CUR);
                readIpv6Packet(psychicCapture, ipTotalLength);
                return;
            }
            if ((header->version & 0x4) != 0x4)
            {
                printf("Not Ipv4\n");
                fclose(psychicCapture);
                free(header);
                fileCorruption();
            }
            if ((header->ipHeaderLength & 0x5) == 0x5)
            {
                /* no Ip Options */
            }
        }
        else if (i == 1)
        {
            header->dscp = buff;
        }
        else if (i < 4)
        {
            hexToShort(&header->ipLength, buff);
            *ipTotalLength = header->ipLength;
        }
        else if (i < 6)
        {
            hexToShort(&header->id, buff);
        }
        else if (i < 8)
        {
            hexToShort(&header->flags, buff);
        }
        else if (i == 8)
        {
            header->ttl = buff;
        }
        else if (i == 9)
        {
            header->protocol = buff;
        }
        else if (i < 12)
        {
            hexToShort(&header->checksum, buff);
        }
        else if (i < 16)
        {
            hexToInt(&header->sourceIp, buff);
        }
        else if (i < 20)
        {
            hexToInt(&header->destIp, buff);
        }
        i++;
    }
    if (header->protocol != 0x11)
    {
        printf("Not UDP Packet.\n");
        fclose(psychicCapture);
        free(header);
        fileCorruption();
    }
    free(header);
}

void
readIpv6Packet(
    FILE * psychicCapture,
    unsigned int *ipTotalLength)
{
    unsigned char   buff = 0;
    int             i = 0;
    ipv6Header     *header = calloc(sizeof(ipv6Header), 1);

    while (i < 40)
    {
        buff = getc(psychicCapture);
        if (i == 0)
        {
            header->version |= (buff << 8);
            header->trafficClass |= buff & 0xf;
            header->trafficClass <<= 4;
        }
        else if (i == 1)
        {
            header->trafficClass |= buff & 0xf0;
            header->flowLabel |= buff & 0xf;
            header->flowLabel <<= 4;
        }
        else if (i < 4)
        {
            header->flowLabel |= buff;
            if (i == 2)
            {
                header->flowLabel <<= 8;
            }
        }
        else if (i < 6)
        {
            hexToShort(&header->payloadLength, buff);
        }
        else if (i < 7)
        {
            header->nextHeader = buff;
        }
        else if (i < 8)
        {
            header->hopLimit = buff;
        }
        else if (i < 24)
        {
            header->source[i - 8] = buff;
        }
        else if (i < 40)
        {
            header->destination[i - 24] = buff;
        }
        i++;
    }
    *ipTotalLength = header->payloadLength;
    if (header->nextHeader != 0x11)
    {
        printf("Not a UDP Packet.\n");
        fclose(psychicCapture);
        free(header);
        fileCorruption();
    }
    free(header);
}

void
readUdpPacket(
    FILE * psychicCapture,
    unsigned int *udpTotalLength,
    unsigned int ipTotalLength)
{
    unsigned char   buff = 0;
    int             i = 0;
    udpHeader      *header = calloc(sizeof(udpHeader), 1);

    while (i < 8)
    {
        buff = getc(psychicCapture);
        if (i < 2)
        {
            hexToShort(&header->sourcePort, buff);
        }
        else if (i < 4)
        {
            hexToShort(&header->destPort, buff);
        }
        else if (i < 6)
        {
            hexToShort(&header->length, buff);
            *udpTotalLength = header->length;
        }
        else if (i < 8)
        {
            hexToShort(&header->checksum, buff);
        }
        i++;
    }
    if (header->length != ipTotalLength - 20)
    {
        printf("Sizes are off.\n");
        fclose(psychicCapture);
        free(header);
        fileCorruption();
    }
    free(header);
}

void
readZergPacket(
    FILE * psychicCapture,
    unsigned int *udpTotalLength)
{
    unsigned char   buff = 0;
    int             i = 0;
    unsigned int    intTotalLength = 0;

    const char     *messageType[] = { "MESSAGE", "STATUS",
        "COMMAND", "GPS"
    };
    zergPacket     *packet = calloc(sizeof(zergPacket), 1);

    while (i < 12)
    {
        buff = getc(psychicCapture);
        if (i == 0)
        {
            packet->type = buff & 0xf;
            buff = buff >> 4;
            packet->version = buff & 0xf;
            if ((packet->version & 0x1) == 0x1)
            {
                printf("Version: 1\n");
            }
            else
            {
                printf("Unknown Version\n");
                fclose(psychicCapture);
                free(packet);
                fileCorruption();
            }
            if (packet->type > 3)
            {
                printf("Unknown Type\n");
                fclose(psychicCapture);
                free(packet);
                fileCorruption();
            }
        }
        else if (i < 4)
        {
            hexToInt(&intTotalLength, buff);
        }
        else if (i < 6)
        {
            hexToShort(&packet->sourceId, buff);
        }
        else if (i < 8)
        {
            hexToShort(&packet->destinationId, buff);
        }
        else if (i < 12)
        {
            hexToInt(&packet->sequenceId, buff);
        }
        i++;
    }
    packet->totalLength |= intTotalLength;
    if (*udpTotalLength - 8 != packet->totalLength)
    {
        printf("Size Irregulatity.\n");
        fclose(psychicCapture);
        free(packet);
        fileCorruption();
    }
    printf("Type: %d %s\n", packet->type, messageType[packet->type]);
    if (packet->type > 3)
    {
        printf("Unkown Type\n");
        fclose(psychicCapture);
        free(packet);
        fileCorruption();
    }
    printf("Size: %d\n", packet->totalLength);
    packet->totalLength -= 12;
    printf("From: %d\n", packet->sourceId);
    printf("To: %d\n", packet->destinationId);
    printf("Sequence: %u\n", packet->sequenceId);
    switch (packet->type)
    {
    case 0:
        readMessage(psychicCapture, packet->totalLength);
        break;
    case 1:
        readStatus(psychicCapture, packet->totalLength);
        break;
    case 2:
        readCommand(psychicCapture);
        break;
    case 3:
        readGPS(psychicCapture);
        break;
    }
    free(packet);
}

void
readMessage(
    FILE * psychicCapture,
    unsigned int payloadLength)
{
    for (unsigned int i = 0; i < payloadLength; i++)
    {
        printf("%c", getc(psychicCapture));
    }
    if (payloadLength == 0)
    {
        fseek(psychicCapture, 6, SEEK_CUR);
        printf("<NO MESSAGE>");
    }
    printf("\n");
}

void
readStatus(
    FILE * psychicCapture,
    unsigned int payloadLength)
{
    unsigned int    intCurrHit = 0;
    unsigned int    intMaxHit = 0;
    unsigned int    i = 0;
    unsigned char   buff = 0;

    const char     *types[] = { "Overmind", "Larva", "Cerebrate",
        "Overlord", "Queen", "Drone", "Zergling", "Lurker",
        "Brooding", "Hydralisk", "Guardian", "Scourge",
        "Ultralisk", "Mutalisk", "Defilier", "Devourer"
    };
    payload        *status = calloc(sizeof(payload), 1);

    for (; i < 12; i++)
    {
        buff = getc(psychicCapture);
        if (i < 3)
        {
            hexToInt(&intCurrHit, buff);
        }
        else if (i == 3)
        {
            status->armor = buff;
        }
        else if (i < 7)
        {
            hexToInt(&intMaxHit, buff);
        }
        else if (i == 7)
        {
            status->type = buff;
        }
        else if (i < 12)
        {
            hexToInt(&status->sSpeed.iSpeed, buff);
        }
    }
    status->currHitPoints |= intCurrHit;
    status->maxHitPoints |= intMaxHit;
    printf("Zerg Type: %d %s\n", status->type, types[status->type]);
    printf("Speed: %f m/s\n", status->sSpeed.fSpeed);
    printf("Health: %d/%d\n", status->currHitPoints, status->maxHitPoints);
    printf("Armor: %d\n", status->armor);
    printf("Name: ");
    while (i < payloadLength)
    {
       	printf("%c", getc(psychicCapture));
        i++;
    }
    printf("\n");
    free(status);
}

void
readCommand(
    FILE * psychicCapture)
{
    unsigned char   buff = 0;
    int             i = 0;

    const char     *commands[] = { "GET_STATUS", "GOTO", "GET_GPS",
        "RESERVED", "RETURN", "SET_GROUP", "STOP", "REPEAT"
    };
    cPayload       *command = calloc(sizeof(cPayload), 1);

    for (; i < 2; i++)
    {
        buff = getc(psychicCapture);
        hexToShort(&command->command, buff);
    }
    if (command->command > 7)
    {
        printf("Bad Command\n");
        fclose(psychicCapture);
        free(command);
        fileCorruption();
    }
    printf("Command: %d %s\n", command->command, commands[command->command]);
    switch (command->command)
    {
    case 1:
        for (; i < 4; i++)
        {
            buff = getc(psychicCapture);
            hexToShort(&command->param1, buff);
        }
        for (; i < 8; i++)
        {
            buff = getc(psychicCapture);
            hexToInt(&command->param2.uiParam2, buff);
        }
        printf("Parameter1: %d\n", command->param1);
        printf("Parameter2: %f\n", command->param2.fParam2);
        break;
    case 5:
        for (; i < 4; i++)
        {
            buff = getc(psychicCapture);
            hexToShort(&command->param1, buff);
        }
        for (; i < 8; i++)
        {
            buff = getc(psychicCapture);
            hexToInt(&command->param2.uiParam2, buff);
        }
        printf("Parameter1: %d\n", command->param1);
        printf("Parameter2: %d\n", command->param2.iParam2);
        break;
    case 7:
        for (; i < 8; i++)
        {
            if (i < 4)
            {
                buff = getc(psychicCapture);
            }
            else
            {
                buff = getc(psychicCapture);
                hexToInt(&command->param2.uiParam2, buff);
            }
        }
        printf("Parameter2: %d\n", command->param2.uiParam2);
        break;
    default:
        fseek(psychicCapture, 4, SEEK_CUR);
        break;
    }
    free(command);
}

void
readGPS(
    FILE * psychicCapture)
{
    unsigned char   buff = 0;
    int             i = 0;
    gpsPayload     *gps = calloc(sizeof(gpsPayload), 1);

    while (i < 32)
    {
        buff = getc(psychicCapture);
        if (i < 8)
        {
            hexToDouble(&gps->longitude.iLong, buff);
        }
        else if (i < 16)
        {
            hexToDouble(&gps->latitude.iLat, buff);
        }
        else if (i < 20)
        {
            hexToInt(&gps->altitude.iAltitude, buff);
        }
        else if (i < 24)
        {
            hexToInt(&gps->bearing.iBearing, buff);
        }
        else if (i < 28)
        {
            hexToInt(&gps->speed.iSpeed, buff);
        }
        else if (i < 32)
        {
            hexToInt(&gps->accuracy.iAccuracy, buff);
        }
        i++;
    }
    printf("Long: %lf \u00B0\n", gps->longitude.dLong);
    printf("Lat: %lf \u00B0\n", gps->latitude.dLat);
    printf("Alt: %f fathoms\n", gps->altitude.fAltitude);
    printf("Bearing: %f \u00B0\n", gps->bearing.fBearing);
    printf("Speed: %f m\\s\n", gps->speed.fSpeed);
    printf("Acc: %f m\n", gps->accuracy.fAccuracy);
    free(gps);
}
