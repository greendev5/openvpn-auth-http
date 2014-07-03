#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "plugin_utils.h"
#include "deferred_handlers.h"

extern int auth_user_pass_verify_handler_func(void *handler_context);

void test_log()
{
    init_plugin_logging(2);
    PLUGIN_LOG("First log");
    PLUGIN_DEBUG("First log");
    PLUGIN_LOG("%s %d", "First log l", 5);
    PLUGIN_DEBUG("%s %d", "First log d", 5);
    clear_plugin_logging();
}

int main(int argc, char **argv)
{

    test_log();
    return 0;

    curl_global_init(CURL_GLOBAL_ALL);

    const char *as[] = {
        "s=1",
        "e=2",
        "dsfsdfsd=",
        "",
        "sfsdfsdf=",
        "q1=cer",
        "auth_control_file=dd.txt",
        NULL
    };

    const char *ex[] = {
        "s",
        "e", "q", "1", "sfsdfsdf",
        NULL
    };

    char **as1 = clone_string_array(as);

    printf("%s\n", openvpn_envp_to_json((const char**)as1, ex));

    deferred_handler_context_t context;
    context.envp = as1;
    strcpy(context.handler_url, "http://127.0.0.1:8001/frontend/test_auth/");

    //write_auth_control_file("dd.txt", 1);
    printf("HHHH: %d\n", auth_user_pass_verify_handler_func(&context));
    free_cloned_string_array(as1);
    curl_global_cleanup();
    return 0;
}