#!/bin/bash

gcc macaroon-test.c base64.o -o macaroon-test -lmacaroons -L./.libs

LD_LIBRARY_PATH=.libs ./macaroon-test < "./test/unit/caveat_v1_1.vtest"