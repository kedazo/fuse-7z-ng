/*
 * This file is part of fuse-7z-ng.
 *
 * fuse-7z-ng is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * fuse-7z-ng is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with fuse-7z-ng.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "config.h"

#include <fuse.h>
#include <fuse_opt.h>

#include <cstdio>
#include <cstring>
#include <unistd.h>

#ifndef PATH_MAX
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
        #include <windows.h>
        #ifndef PATH_MAX
            #define PATH_MAX MAX_PATH
        #endif
    #else
        #include <linux/limits.h>
    #endif
#endif

#include "logger.h"
#include "fuse_functions.h"

/**
 * Print usage information
 */
void print_usage()
{
    printf ("usage: fuse-7z-ng [options] <zip-file> <mountpoint>\n\n"
            "general options:\n"
            "    -o opt,[opt...]        mount options\n"
            "    -h   --help            print help\n"
            "    -V   --version         print version\n"
            "    -r   -o ro             open archive in read-only mode\n"
            "    -f                     don't detach from terminal\n"
            "    -d                     turn on debugging, also implies -f\n"
            "\n");
}

/**
 * Print version information (fuse-zip and FUSE library)
 */
void print_version()
{
    printf ("fuse-7z-ng version: %s\n", VERSION);
}

/**
 * Parameters for command-line argument processing function
 */
struct fuse7z_param
{
    // help shown
    int help;
    // version information shown
    int version;
    // number of string arguments
    int strArgCount;
    // zip file name
    const char *fileName;
    // verbosity
    int verbose;
    int automake;
    char mountpoint[4096];
} param;

enum key:uint8_t{
 KEY_HELP=0,
 KEY_VERSION=1,
 KEY_AUTO=2,
 KEY_SYSLOG=3
};

static const struct fuse_opt fuse7z_opts[] =
{
    FUSE_OPT_KEY ("-h", KEY_HELP),
    FUSE_OPT_KEY ("--help", KEY_HELP),
    FUSE_OPT_KEY ("-V", KEY_VERSION),
    FUSE_OPT_KEY ("--version", KEY_VERSION),
    FUSE_OPT_KEY ("--automount", KEY_AUTO),
    FUSE_OPT_KEY ("--syslog", KEY_SYSLOG),
    FUSE_OPT_KEY (nullptr, 0)
};

/**
 * Function to process arguments (called from fuse_opt_parse).
 *
 * @param data  Pointer to fuse7z_param structure
 * @param arg is the whole argument or option
 * @param key determines why the processing function was called
 * @param outargs the current output argument list
 * @return -1 on error, 0 if arg is to be discarded, 1 if arg should be kept
 */
static int
process_arg (
        struct fuse7z_param     *param,
        const char              *arg,
        int                      key,
        struct fuse_args        *outargs)
{
    // 'magic' fuse_opt_proc return codes
    #if !defined(KEEP)
    const static int KEEP = 1;
    #endif
    #if !defined(DISCARD)
    const static int DISCARD = 0;
    #endif
    #if !defined(ERROR)
    const static int ERROR = -1;
    #endif

    switch (key) {
        case KEY_HELP:
            print_usage();
            param->help = 1;
            return DISCARD;

        case KEY_VERSION:
            print_version();
            param->version = 1;
            return KEEP;

        case KEY_AUTO:
            param->automake = 1;
            return DISCARD;

        case KEY_SYSLOG:
            Logger::instance ().enableSyslog (true);
            return DISCARD;

        case FUSE_OPT_KEY_NONOPT:
            ++param->strArgCount;
            switch (param->strArgCount) {
                case 1: {
                            param->fileName = arg;
                            return DISCARD;
                        }
                case 2: {
                            strcpy(param->mountpoint, arg);
                            return KEEP;
                        }
                default:
                        fprintf(stderr, "only two arguments allowed: filename and mountpoint\n");
                        return ERROR;
            }

        default:
            return KEEP;
    }
}

int
main (int argc, char **argv)
{
    struct fuse_args args = FUSE_ARGS_INIT (argc, argv);
    void * data;

    char *mountpoint;
    int multithreaded;  // this flag ignored because libzip does not supports multithreading
    int res;
    if (fuse_opt_parse (&args, &param, fuse7z_opts, (fuse_opt_proc_t)process_arg))
    {
        fuse_opt_free_args(&args);
        return 2;
    }

    // if all work is done inside options parsing...
    if (param.help || param.version)
    {
        fuse_opt_free_args(&args);
        return 3;
    }

    char cwd[PATH_MAX+1];
    if (getcwd (cwd, PATH_MAX) == nullptr)
    {
        return 4;
    }

    // check if no file name passed
    if (param.fileName == nullptr)
    {
        print_usage();
        fuse_opt_free_args(&args);
        return 4;
    }

    data = fuse7z_initlib (param.fileName, cwd);
    if (! data )
    {
        fuse_opt_free_args(&args);
        return 5;
    }

    if (param.automake)
    {
        #if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
            mkdir(param.mountpoint);
        #else
            mkdir(param.mountpoint, 0750);
        #endif
        
    }

    struct fuse_operations fuse7z_oper = {0};
    fuse7z_oper.init = fuse7z_init;
    fuse7z_oper.destroy = fuse7z_destroy;
    fuse7z_oper.readdir = fuse7z_readdir;
    fuse7z_oper.getattr = fuse7z_getattr;
    fuse7z_oper.statfs = fuse7z_statfs;
    fuse7z_oper.open = fuse7z_open;
    fuse7z_oper.read = fuse7z_read;
    fuse7z_oper.write = fuse7z_write;
    fuse7z_oper.release = fuse7z_release;
    fuse7z_oper.unlink = fuse7z_unlink;
    fuse7z_oper.rmdir = fuse7z_rmdir;
    fuse7z_oper.mkdir = fuse7z_mkdir;
    fuse7z_oper.rename = fuse7z_rename;
    fuse7z_oper.create = fuse7z_create;
    fuse7z_oper.chmod = fuse7z_chmod;
    fuse7z_oper.chown = fuse7z_chown;
    fuse7z_oper.flush = fuse7z_flush;
    fuse7z_oper.fsync = fuse7z_fsync;
    fuse7z_oper.fsyncdir = fuse7z_fsyncdir;
    fuse7z_oper.opendir = fuse7z_opendir;
    fuse7z_oper.releasedir = fuse7z_releasedir;
    fuse7z_oper.access = fuse7z_access;
    fuse7z_oper.utimens = fuse7z_utimens;
    #if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
        #undef ftruncate
    #endif
    fuse7z_oper.ftruncate = fuse7z_ftruncate;
    fuse7z_oper.truncate = fuse7z_truncate;
    fuse7z_oper.setxattr = fuse7z_setxattr;
    fuse7z_oper.getxattr = fuse7z_getxattr;
    fuse7z_oper.listxattr = fuse7z_listxattr;
    fuse7z_oper.removexattr = fuse7z_removexattr;
    #if FUSE_VERSION >= 28
    // don't allow nullptr path
    fuse7z_oper.flag_nullpath_ok = 0;
    #endif
    struct fuse * fuse = fuse_setup (
                args.argc, args.argv,
                &fuse7z_oper, sizeof(fuse7z_oper),
                &mountpoint, &multithreaded, data);

    fuse_opt_free_args(&args);
    if (fuse == nullptr)
    {
        return 7;
    }

    res = fuse_loop(fuse);

    fuse_teardown(fuse, mountpoint);

    #if 0
    // automake must not do 'remove' operation
    if (param.automake) {
        rmdir(param.mountpoint);
    }
    #endif

    return (res == 0) ? 0 : 8;
}

