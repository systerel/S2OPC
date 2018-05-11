/*
 *  Copyright (C) 2018 Systerel and others.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int LLVMFuzzerTestOneInput(const unsigned char* data, size_t size);

static bool read_all(FILE* fd, unsigned char* data, size_t len)
{
    size_t n = 0;

    while (true)
    {
        size_t read = fread(data + n, sizeof(unsigned char), len - n, fd);
        n += read;

        if (read == 0 || n == len)
        {
            break;
        }
    }

    int err = ferror(fd);

    if (err != 0)
    {
        fprintf(stderr, "Error while reading: %s\n", strerror(err));
        return false;
    }

    return true;
}

static bool fuzz_file(FILE* fd)
{
    if (fseek(fd, 0, SEEK_END) == -1)
    {
        perror("Cannot seek to the end of file");
        return false;
    }

    long len = ftell(fd);

    if (len == -1)
    {
        perror("ftell");
        return false;
    }

    if (fseek(fd, 0, SEEK_SET) == -1)
    {
        perror("Cannot rewind the file");
        return false;
    }

    unsigned char* buf = (unsigned char*) malloc((size_t) len);

    if (buf == NULL)
    {
        fprintf(stderr, "Memory allocation failure\n");
        return false;
    }

    if (!read_all(fd, buf, (size_t) len))
    {
        return false;
    }

    LLVMFuzzerTestOneInput(buf, (size_t) len);
    free(buf);

    fprintf(stderr, "Done (%ld bytes)\n", len);

    return true;
}

int main(int argc, char** argv)
{
    fprintf(stderr, "Running the fuzzing function on %d inputs\n", argc - 1);

    for (int i = 1; i < argc; i++)
    {
        const char* filename = argv[i];

        fprintf(stderr, "Running: %s\n", filename);

        FILE* fd = fopen(filename, "r");

        if (fd == NULL)
        {
            fprintf(stderr, "Cannot open %s: %s\n", filename, strerror(errno));
            return 1;
        }

        bool ok = fuzz_file(fd);
        fclose(fd);

        if (!ok)
        {
            return 1;
        }
    }
}
