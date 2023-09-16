#!/bin/sh
set -ex

CLIENT_CFLAGS=$(xmlrpc-c-config libwww-client --cflags)
CLIENT_LIBS=$(xmlrpc-c-config libwww-client --libs)
gcc $CLIENT_CFLAGS -o out main.c $CLIENT_LIBS
