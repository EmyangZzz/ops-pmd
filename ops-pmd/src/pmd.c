/*
 *  (c) Copyright 2015 Hewlett Packard Enterprise Development LP
 *  Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014 Nicira, Inc.
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

/************************************************************************//**
 * @ingroup ops-pmd
 *
 * @file
 * Source file for the platform Pluggable Module daemon
 ***************************************************************************/

#define _GNU_SOURCE
#include <config.h>

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <command-line.h>
#include <daemon.h>
#include <dirs.h>
#include <fatal-signal.h>
#include <ovsdb-idl.h>
#include <poll-loop.h>
#include <unixctl.h>
#include <util.h>
#include <dynamic-string.h>
#include <vswitch-idl.h>
#include <coverage.h>

#include "pmd.h"

VLOG_DEFINE_THIS_MODULE(ops_pmd);

COVERAGE_DEFINE(pmd_reconfigure);

static unixctl_cb_func pmd_unixctl_dump;
#ifdef PLATFORM_SIMULATION
static unixctl_cb_func pmd_unixctl_sim;
#endif
static unixctl_cb_func ops_pmd_exit;

static char *parse_options(int argc, char *argv[], char **unixctl_path);
OVS_NO_RETURN static void usage(void);
static void pmd_print_version(void);

static char *program_version = "0.02";

extern struct ovsdb_idl *idl;
extern void pmd_reconfigure(struct ovsdb_idl *idl);
extern int pmd_sim_insert(const char *name, const char *file, struct ds *ds);
extern int pmd_sim_remove(const char *name, struct ds *ds);

static void
pmd_init(const char *remote)
{
    pm_config_init();
    pm_ovsdb_if_init(remote);
    unixctl_command_register("ops-pmd/dump", "", 0, 2,
                             pmd_unixctl_dump, NULL);

#ifdef PLATFORM_SIMULATION
    unixctl_command_register("ops-pmd/sim", "", 2, 3,
                             pmd_unixctl_sim, NULL);
#endif
}

static void
pmd_exit(void)
{
    ovsdb_idl_destroy(idl);
}

static void
pmd_run(void)
{
    int rc;

    ovsdb_idl_run(idl);

    if (ovsdb_idl_is_lock_contended(idl)) {
        static struct vlog_rate_limit rl = VLOG_RATE_LIMIT_INIT(1, 1);

        VLOG_ERR_RL(&rl, "another ops-pmd process is running, "
                    "disabling this process until it goes away");

        return;
    } else if (!ovsdb_idl_has_lock(idl)) {
        return;
    }

    //进程数据库更改。
    pmd_reconfigure(idl);

    //扫描可插拔模块的当前状态。
    rc = pm_read_state();
    if (0 != rc) {
        VLOG_ERR_ONCE("Failed to read pluggable module state, rc=%d\n", rc);
    }

    //更新OVSDB。
    pm_ovsdb_update();

    daemonize_complete();
    vlog_enable_async();
    VLOG_INFO_ONCE("%s (OpenSwitch pmd) %s", program_name, program_version);
}

static void
pmd_wait(void)
{
    ovsdb_idl_wait(idl);

    //定期唤醒可插拔模块检测。
    poll_timer_wait_at(PM_INTERVAL, __FUNCTION__);
}

#ifdef PLATFORM_SIMULATION
static void
pmd_unixctl_sim(struct unixctl_conn *conn, int argc,
                const char *argv[], void *aux OVS_UNUSED)
{
    struct ds ds = DS_EMPTY_INITIALIZER;

    int rc = 0;
    const char *interface = argv[1];

    /* 用法:
        ops-pmd/sim <interface> insert <file>
        ops-pmd/sim <interface> remove
    */
    if (4 == argc && strcmp("insert", argv[2]) == 0) {
        rc = pmd_sim_insert(interface, argv[3], &ds);
    } else if (3 == argc && strcmp("remove", argv[2]) == 0) {
        rc = pmd_sim_remove(interface, &ds);
    } else {
        rc = -1;
        ds_put_cstr(&ds, "Invalid usage: ... ops-pmd/sim <interface> [insert <file> | remove]");
        return;
    }

    if (rc < 0) {
        unixctl_command_reply_error(conn, ds_cstr(&ds));
    } else {
        unixctl_command_reply(conn, ds_cstr(&ds));
    }

    ds_destroy(&ds);
}
#endif

static void
pmd_unixctl_dump(struct unixctl_conn *conn, int argc OVS_UNUSED,
                 const char *argv[] OVS_UNUSED, void *aux OVS_UNUSED)
{
    struct ds ds = DS_EMPTY_INITIALIZER;

    pm_debug_dump(&ds, argc, argv);

    unixctl_command_reply(conn, ds_cstr(&ds));
    ds_destroy(&ds);
}

int
main(int argc, char *argv[])
{
    char *unixctl_path = NULL;
    struct unixctl_server *unixctl;
    char *remote;
    bool exiting;
    int retval;

    set_program_name(argv[0]);

    proctitle_init(argc, argv);
    remote = parse_options(argc, argv, &unixctl_path);
    fatal_ignore_sigpipe();

    ovsrec_init();

    daemonize_start();

    retval = unixctl_server_create(unixctl_path, &unixctl);
    if (retval) {
        exit(EXIT_FAILURE);
    }
    unixctl_command_register("exit", "", 0, 0, ops_pmd_exit, &exiting);

    pmd_init(remote);
    free(remote);

    exiting = false;
    while (!exiting) {
        pmd_run();
        unixctl_server_run(unixctl);

        pmd_wait();
        unixctl_server_wait(unixctl);

        if (exiting) {
            poll_immediate_wake();
        }
        poll_block();
    }
    pmd_exit();
    unixctl_server_destroy(unixctl);

    return 0;
}

static char *
parse_options(int argc, char *argv[], char **unixctl_pathp)
{
    enum {
        OPT_UNIXCTL = UCHAR_MAX + 1,
        VLOG_OPTION_ENUMS,
        DAEMON_OPTION_ENUMS,
    };
    static const struct option long_options[] = {
        {"help",        no_argument, NULL, 'h'},
        {"version",     no_argument, NULL, 'V'},
        {"unixctl",     required_argument, NULL, OPT_UNIXCTL},
        DAEMON_LONG_OPTIONS,
        VLOG_LONG_OPTIONS,
        {NULL, 0, NULL, 0},
    };
    char *short_options = long_options_to_short_options(long_options);

    for (;;) {
        int c;

        c = getopt_long(argc, argv, short_options, long_options, NULL);
        if (c == -1) {
            break;
        }

        switch (c) {
        case 'h':
            usage();

        case 'V':
            pmd_print_version();
            exit(EXIT_SUCCESS);

        case OPT_UNIXCTL:
            *unixctl_pathp = optarg;
            break;

        VLOG_OPTION_HANDLERS
        DAEMON_OPTION_HANDLERS

        case '?':
            exit(EXIT_FAILURE);

        default:
            abort();
        }
    }
    free(short_options);

    argc -= optind;
    argv += optind;

    switch (argc) {
    case 0:
        return xasprintf("unix:%s/db.sock", ovs_rundir());

    case 1:
        return xstrdup(argv[0]);

    default:
        VLOG_FATAL("at most one non-option argument accepted; "
                   "use --help for usage");
    }
}

static void
usage(void)
{
    printf("%s: OpenSwitch pluggable module daemon (pmd)\n"
           "usage: %s [OPTIONS] [DATABASE]\n"
           "where DATABASE is a socket on which ovsdb-server is listening\n"
           "      (default: \"unix:%s/db.sock\").\n",
           program_name, program_name, ovs_rundir());
    daemon_usage();
    vlog_usage();
    printf("\nOther options:\n"
           "  --unixctl=SOCKET        override default control socket name\n"
           "  -h, --help              display this help message\n"
           "  -V, --version           display version information\n");
    exit(EXIT_SUCCESS);
}

static void
ops_pmd_exit(struct unixctl_conn *conn, int argc OVS_UNUSED,
                  const char *argv[] OVS_UNUSED, void *exiting_)
{
    bool *exiting = exiting_;
    *exiting = true;
    unixctl_command_reply(conn, NULL);
}

static void
pmd_print_version(void)
{
    printf("%s version %s\n", program_name, program_version);
}
