/*
 *  (c) Copyright 2015 Hewlett Packard Enterprise Development LP
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License. You may obtain
 *  a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 *  License for the specific language governing permissions and limitations
 *  under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

unsigned char
convert(char value)
{
    value = toupper(value);

    if (value >= '0' && value <= '9') {
        return((unsigned char)(value - '0'));
    }
    if (value >= 'A' && value <= 'F') {
        return((unsigned char)(value - 'A')+0xA);
    }

    fprintf(stderr, "Invalid character: %c\n", value);
    exit(1);
}

int
main(int argc, char **argv)
{
    FILE *fp;
    char buf[2];
    int count = 0;

    fp = fopen(argv[1], "r");

    if (fp == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", argv[1]);
        exit(1);
    }

    while (!feof(fp) && !ferror(fp)) {
        unsigned char data;
        fread(buf, 2, 1, fp);
        if (buf[0] == '\n') {
            break;
        }
        data = (convert(buf[0]) << 4) + convert(buf[1]);
        write(1, &data, 1);
        count++;
    }

    fclose(fp);

    fprintf(stderr, "Count is %d\n", count);

    exit(0);
}
