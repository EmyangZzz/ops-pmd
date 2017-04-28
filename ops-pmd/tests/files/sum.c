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

void
sum(unsigned char *data, int start, int end, int offset)
{
    unsigned char value = 0;
    int idx;

    for (idx = start; idx <= end; idx++) {
        value += data[idx];
    }

    printf("%d vs. %d\n", value, data[offset]);
}

int
main(int argc, char **argv)
{
    FILE *fp;
    unsigned char buf[128];
    int count = 0;

    fp = fopen(argv[1], "r");

    if (fp == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", argv[1]);
        exit(1);
    }

    fread(buf, 128, 1, fp);
    fclose(fp);

    sum(buf, 0, 62, 63);
    sum(buf, 64, 94, 95);

    exit(0);
}
