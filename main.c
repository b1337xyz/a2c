#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

static char * basename(char * path)
{
    char *delimiter = "/";
    char *token, *lastToken;
    token = strtok(path, delimiter);
    while (token != NULL) {
        lastToken = token;
        token = strtok(NULL, delimiter);
    }
    return lastToken;
}

static char * get_torrent_name(xmlrpc_env * envP, xmlrpc_value * structP)
{
    char *name;
    xmlrpc_decompose_value(envP, structP,
                           "{s:{s:s,*},*}",
                           "info", "name", &name);
    return name;
}

int main(int const argc, const char ** const argv)
{
    xmlrpc_env env;
    xmlrpc_value * resP;
    const char * const serverUrl = "http://localhost:6800/rpc";
    const char * const methodName = "aria2.tellWaiting";
    const char * stateName;
    unsigned int total_waiting;

    xmlrpc_env_init(&env);
    xmlrpc_client_init2(&env, XMLRPC_CLIENT_NO_FLAGS, NAME, VERSION, NULL, 0);
    die(&env);
    printf("server: %s\nmethod: %s\n", serverUrl, methodName);
    resP = xmlrpc_client_call(&env, serverUrl, methodName, "(ii)",
                              (xmlrpc_int32) 0, (xmlrpc_int32) 99);
    die(&env);
    total_waiting = xmlrpc_array_size(&env, resP);
    die(&env);
    printf("waiting: %d\n", total_waiting);
    for (unsigned int i = 0; i < total_waiting; i++) {
        xmlrpc_value * array_elementP;
        xmlrpc_value * bittorrent;
        xmlrpc_value * files;
        xmlrpc_value * first_file;
        char *dir;
        char *gid;
        char *name;
        char *path;
        xmlrpc_array_read_item(&env, resP, i, &array_elementP);
        int is_torrent = xmlrpc_struct_has_key(&env,
                                               array_elementP, 
                                               "qbittorrent");

        if (is_torrent == 0) {
            xmlrpc_decompose_value(&env, array_elementP,
                                   "{s:s,s:s,s:A,s:S,*}",
                                   "dir", &dir,
                                   "gid", &gid,
                                   "files", &files,
                                   "bittorrent", &bittorrent
                                  );
        } else {
            xmlrpc_decompose_value(&env, array_elementP,
                                   "{s:s,s:s,s:A,*}",
                                   "dir", &dir,
                                   "gid", &gid,
                                   "files", &files
                                  );
        }

        xmlrpc_array_read_item(&env, files, 0, &first_file); 
        xmlrpc_decompose_value(&env, first_file,
                               "{s:s,*}",
                               "path", &path);

        if (is_torrent == 0) {
            name = get_torrent_name(&env, bittorrent);
        } else {
            name = basename(path);
        }

        printf("GID %s\n", gid);
        printf("DIR %s\n", dir);
        printf("NAME %s\n", name);
        die(&env);

        /* Dispose of our result value. */
        xmlrpc_DECREF(array_elementP);
        xmlrpc_DECREF(bittorrent);
        xmlrpc_DECREF(files);
        xmlrpc_DECREF(first_file);
    }

    /* Dispose of our result value. */
    xmlrpc_DECREF(resP);

    /* Shutdown our XML-RPC client library. */
    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();
    return 0;
}

