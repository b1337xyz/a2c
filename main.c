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

static const char * basename(const char * path)
{
    const char *lastSlash = NULL;

    for (const char *ptr = path; *ptr != '\0'; ptr++) {
        if (*ptr == '/') {
            lastSlash = ptr;
        }
    }

    return lastSlash == NULL ? path : lastSlash + 1;
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
        const char *dir;
        const char *gid;
        const char *name;
        const char *path;
        xmlrpc_array_read_item(&env, resP, i, &array_elementP);
        int is_torrent = xmlrpc_struct_has_key(&env,
                                               array_elementP, 
                                               "qbittorrent");

        if (is_torrent == 1) {
            xmlrpc_decompose_value(&env, array_elementP,
                                   "{s:s,s:s,s:A,s:S,*}",
                                   "dir", &dir,
                                   "gid", &gid,
                                   "files", &files,
                                   "bittorrent", &bittorrent
                                  );
        } else {
            xmlrpc_value * uris;
            xmlrpc_decompose_value(&env, array_elementP,
                                   "{s:s,s:s,s:A,*}",
                                   "dir", &dir,
                                   "gid", &gid,
                                   "files", &files
                                  );
        }

        xmlrpc_array_read_item(&env, files, 0, &first_file); 
        if (xmlrpc_struct_has_key(&env, first_file, "path") == 1) {
            xmlrpc_decompose_value(&env, first_file,
                                   "{s:s,*}",
                                   "path", &path);
        } else {
            xmlrpc_value * uris;
            xmlrpc_value * first_uri;
            xmlrpc_decompose_value(&env, first_file,
                                   "{s:A,*}",
                                   "uris", &uris);

            if (xmlrpc_array_size(&env, uris) > 0) {
                xmlrpc_array_read_item(&env, uris, 0, &first_uri); 
                xmlrpc_decompose_value(&env, first_uri,
                                       "{s:s,*}",
                                       "uri", &path);
                xmlrpc_DECREF(uris);
                xmlrpc_DECREF(first_uri);
            }
        }

        if (is_torrent == 1) {
            name = get_torrent_name(&env, bittorrent);
        } else {
            name = basename(path);
        }

        printf("GID %s\n", gid);
        printf("DIR %s\n", dir);
        printf("NAME %s\n", name);
        printf("PATH %s\n", path);
        die(&env);

        /* Dispose of our result value. */
        xmlrpc_DECREF(array_elementP);
        xmlrpc_DECREF(files);
        xmlrpc_DECREF(first_file);
        if (is_torrent == 1) xmlrpc_DECREF(bittorrent);
    }

    /* Dispose of our result value. */
    xmlrpc_DECREF(resP);

    /* Shutdown our XML-RPC client library. */
    xmlrpc_env_clean(&env);
    xmlrpc_client_cleanup();
    return 0;
}

