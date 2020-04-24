/* Copyright (c) 2016, Robert Escriva
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of this project nor the names of its contributors may
 *       be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define _WITH_GETLINE

/* C */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* macaroons */
#include "macaroons.h"
#include "macaroons-inner.h"
#include "slice.h"
#include "base64.h"

#define STRLENOF(X) (sizeof(X) - 1)

#define VERSION "version "
#define AUTHORIZED "authorized"
#define UNAUTHORIZED "unauthorized"
#define KEY "key "
#define EXACT "exact "
#define GENERAL "general "

int
parse_version(const char* line)
{
    const char* ptr = line + STRLENOF(VERSION);
    const char* const end = line + strlen(line);

    while (ptr < end)
    {
        const char* tmp = strchr(ptr, ' ');
        tmp = tmp ? tmp : end;

        if (strncmp(ptr, "1", tmp - ptr) != 0 &&
            strncmp(ptr, "2", tmp - ptr) != 0 &&
            strncmp(ptr, "2j", tmp - ptr) != 0)
        {
            fprintf(stderr, "version %.*s not supported\n", (int)(tmp - ptr), ptr);
            return -1;
        }

        ptr = tmp + 1;
    }

    return 0;
}

int
parse_key(const char* line, unsigned char** key, size_t* key_sz)
{
    const char* ptr = line + STRLENOF(KEY);
    *key_sz = strlen(ptr);
    *key = malloc(*key_sz);

    if (!*key)
    {
        return -1;
    }

    memmove(*key, ptr, *key_sz);
    return 0;
}

int
parse_exact_caveat(const char* line, struct macaroon_verifier* V)
{
    enum macaroon_returncode err;
    const char* ptr = line + STRLENOF(EXACT);
    size_t ptr_sz = strlen(ptr);

    if (macaroon_verifier_satisfy_exact(V, (const unsigned char*)ptr, ptr_sz, &err) < 0)
    {
        fprintf(stderr, "could not add exact caveat: %s\n", macaroon_error(err));
        return -1;
    }

    return 0;
}

int
parse_macaroon(const char* line, struct macaroon*** macaroons, size_t* macaroons_sz)
{
    size_t buf_sz = strlen(line);
    unsigned char* buf = malloc(buf_sz);
    struct macaroon** tmp;

    if (!buf)
    {
        return -1;
    }

    int rc = b64_pton(line, buf, buf_sz);

    if (rc < 0)
    {
        fprintf(stderr, "could not unwrap serialized macaroon\n");
        free(buf);
        return -1;
    }

    enum macaroon_returncode err;
    struct macaroon* M = macaroon_deserialize(buf, rc, &err);
    free(buf);

    if (!M)
    {
        fprintf(stderr, "could not deserialize macaroon: %s\n", macaroon_error(err));
        return -1;
    }

    ++*macaroons_sz;
    tmp = realloc(*macaroons, *macaroons_sz * sizeof(struct macaroon*));

    if (!tmp)
    {
        return -1;
    }

    *macaroons = tmp;
    (*macaroons)[*macaroons_sz - 1] = M;
    return 0;
}

int
main(int argc, const char* argv[])
{
    char* line = NULL;
    size_t line_sz = 0;
    int authorized = 0;
    unsigned char* key = NULL;
    size_t key_sz = 0;
    struct macaroon_verifier* V = NULL;
    struct macaroon** macaroons = NULL;
    size_t macaroons_sz = 0;
    size_t i = 0;
    int ret = EXIT_SUCCESS;

    if (!(V = macaroon_verifier_create()))
    {
        fprintf(stderr, "could not create verifier: %s\n", strerror(ferror(stdin)));
        abort();
    }

    while (1)
    {
        ssize_t amt = getline(&line, &line_sz, stdin);

        if (amt < 0)
        {
            if (feof(stdin) != 0)
            {
                break;
            }

            fprintf(stderr, "could not read from stdin: %s\n", strerror(ferror(stdin)));
            abort();
        }

        if (!line || amt == 0 || *line == '\n' || *line == '#')
        {
            continue;
        }

        line[amt - 1] = '\0';

        if (strncmp(line, VERSION, STRLENOF(VERSION)) == 0)
        {
            if (parse_version(line) < 0)
            {
                abort();
            }
        }
        else if (strcmp(line, AUTHORIZED) == 0)
        {
            authorized = 1;
        }
        else if (strcmp(line, UNAUTHORIZED) == 0)
        {
            authorized = 0;
        }
        else if (strncmp(line, KEY, STRLENOF(KEY)) == 0)
        {
            if (parse_key(line, &key, &key_sz) < 0)
            {
                abort();
            }
        }
        else if (strncmp(line, EXACT, STRLENOF(EXACT)) == 0)
        {
            if (parse_exact_caveat(line, V) < 0)
            {
                abort();
            }
        }
        else if (strncmp(line, GENERAL, STRLENOF(GENERAL)) == 0)
        {
            abort();
        }
        else if (parse_macaroon(line, &macaroons, &macaroons_sz) < 0)
        {
            abort();
        }
    }

    if (macaroons_sz == 0)
    {
        fprintf(stderr, "no macaroons provided\n");
        abort();
    }

    // create a new macaroon (which is a replica of the one in caveat_v1_1.vtest)

    struct macaroon* M = macaroons[0];
    struct macaroon* N = NULL;
    enum macaroon_returncode err;
    size_t data_sz;
    unsigned char* data;

    //characteristics of the new macaroon
    const unsigned char* N_key = "this is the key";
    size_t N_key_sz = strlen(N_key);

    const unsigned char* N_location = "http://example.org/";
    size_t N_location_sz = strlen(N_location);

    const unsigned char* N_identifier = "keyid";
    size_t N_identifier_sz = strlen(N_identifier);

    const char* N_predicate = "account = 3735928559";
    size_t N_predicate_sz = strlen(N_predicate);

    const char* N_predicate_2 = "ip = 8.8.8.8";
    size_t N_predicate_sz_2 = strlen(N_predicate_2);

    N = macaroon_create(N_location, N_location_sz, N_key, N_key_sz, N_identifier, N_identifier_sz, &err);

    // N = macaroon_add_first_party_caveat(N, N_predicate, N_predicate_sz, &err);
    N = macaroon_add_first_party_caveat(N, N_predicate_2, N_predicate_sz_2, &err);

    // pretty print the original and new macaroon
    printf("\n");
    data_sz = macaroon_inspect_size_hint(M);
    data = malloc(data_sz);
    macaroon_inspect(M, data, data_sz, &err);
    printf("%s\n\n", data);

    data_sz = macaroon_inspect_size_hint(N);
    data = malloc(data_sz);
    macaroon_inspect(N, data, data_sz, &err);
    printf("%s\n\n", data);

    // serialise the new macaroon and compare with the old macaroon
    size_t N_buf_sz = macaroon_serialize_size_hint(N, MACAROON_V1);
    unsigned char* N_buf = malloc(N_buf_sz);

    size_t M_buf_sz = macaroon_serialize_size_hint(M, MACAROON_V1);
    unsigned char* M_buf = malloc(M_buf_sz);

    macaroon_serialize(N, MACAROON_V1, N_buf, N_buf_sz, &err);
    macaroon_serialize(M, MACAROON_V1, M_buf, M_buf_sz, &err);

    printf("N serialized:\t%s\n", N_buf);
    printf("M serialized:\t%s\n", M_buf);

    // show this is the same as the serialised string in caveat_v1_1.vtest
    M_buf_sz = strlen(line);
    M_buf = malloc(M_buf_sz);
    b64_pton(line, M_buf, M_buf_sz);  // unsure why or where the base conversion happens
    printf("M serialized:\t%s\n", M_buf);

    // deserialise and verify the token
    N = macaroon_deserialize(N_buf, N_buf_sz, &err);

    // pretty print for quick verification
    printf("\n");
    data_sz = macaroon_inspect_size_hint(N);
    data = malloc(data_sz);
    macaroon_inspect(N, data, data_sz, &err);
    printf("%s\n\n", data);

    // construct a verifier
    V = macaroon_verifier_create();

    int satisfy;

    satisfy = macaroon_verifier_satisfy_exact(V, N_predicate, N_predicate_sz, &err);
    printf("satisfy\t= %d\n", satisfy);
    printf("err:\t= %d\n", err);
    satisfy = macaroon_verifier_satisfy_exact(V, N_predicate_2, N_predicate_sz_2, &err);
    printf("satisfy\t= %d\n", satisfy);
    printf("err:\t= %d\n", err);

    int verify = macaroon_verify(V, N, N_key, N_key_sz, NULL, 0, &err);
    printf("verify\t= %d\n", verify);
    printf("err:\t= %d\n", err);

    // cleanup!
    if (line)
    {
        free(line);
    }

    if (key)
    {
        free(key);
    }

    for (i = 0; i < macaroons_sz; ++i)
    {
        if (macaroons[i])
        {
            macaroon_destroy(macaroons[i]);
        }
    }

    if (macaroons)
    {
        free(macaroons);
    }

    if (V)
    {
        macaroon_verifier_destroy(V);
    }

    (void) argc;
    (void) argv;

    return ret;
}
