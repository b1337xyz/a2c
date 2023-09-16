#include <stdlib.h>
#include <stdio.h>
#include <xmlrpc-c/base.h>
#include <xmlrpc-c/client.h>

#define NAME "aria2 rpc client"
#define VERSION "1.0"

static void die(xmlrpc_env * const envP) {
    if (envP->fault_occurred) {
        fprintf(stderr, "ERROR: %s (%d)\n",
                envP->fault_string, envP->fault_code);
        exit(1);
    }
}

static char * get_torrent_name(xmlrpc_env * const envP,
                               xmlrpc_value * const structP)
{
        
    xmlrpc_value * infoP;
    char *name;
    xmlrpc_decompose_value(envP, structP,
                           "{s:S,*}",
                           "info", &infoP
                          );

    xmlrpc_decompose_value(envP, infoP,
                           "{s:s,*}",
                           "name", &name
                          );

    xmlrpc_DECREF(structP);
    xmlrpc_DECREF(infoP);
    return name;
}


int main(int const argc, const char ** const argv)
{
    xmlrpc_env env;
    xmlrpc_value * resP;
    const char * const serverUrl = "http://localhost:6800/rpc";
    const char * const methodName = "aria2.tellWaiting";
    const char * stateName;
    unsigned int total_process;

    xmlrpc_env_init(&env);
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0);
    die(&env);
    printf("server: %s\nmethod: %s\n", serverUrl, methodName);
    resP = xmlrpc_client_call(&env, serverUrl, methodName, "(ii)",
                              (xmlrpc_int32) 0, (xmlrpc_int32) 99);
    die(&env);
    total_process = xmlrpc_array_size(&env, resP);
    die(&env);
    printf("total: %d\n", total_process);
    for (unsigned int i = 0; i < total_process; i++) {
        xmlrpc_value * array_elementP;
        xmlrpc_value * bittorrentP;
        xmlrpc_value * infoP;
        char *dir;
        char *gid;
        char *name;
        xmlrpc_array_read_item(&env, resP, i, &array_elementP);
        xmlrpc_decompose_value(&env, array_elementP,
                               "{s:s,s:s,s:S,*}",
                               "dir", &dir,
                               "gid", &gid,
                               "bittorrent", &bittorrentP
                               );

        name = get_torrent_name(&env, bittorrentP);
        printf("GID %s\n", gid);
        printf("DIR %s\n", dir);
        printf("NAME %s\n", name);
        die(&env);

        /* Dispose of our result value. */
        xmlrpc_DECREF(array_elementP);

    }

    /* Dispose of our result value. */
    xmlrpc_DECREF(resP);

    /* Shutdown our XML-RPC client library. */
    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();
    return 0;
}

