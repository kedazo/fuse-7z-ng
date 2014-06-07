#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <linux/limits.h>
#include <sys/time.h>
#include <unistd.h>
#include <cstring>

#define FUSE_USE_VERSION 28
#include <fuse.h>
#include <fuse_opt.h>

void * fuse7z_initlib(char const * archive, char const * cwd);
void *fuse7z_init(struct fuse_conn_info *conn);
void fuse7z_destroy(void *data);
int fuse7z_getattr(const char *path, struct stat *stbuf);
int fuse7z_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int fuse7z_statfs(const char *path, struct statvfs *buf);
int fuse7z_open(const char *path, struct fuse_file_info *fi);
int fuse7z_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int fuse7z_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int fuse7z_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int fuse7z_release (const char *path, struct fuse_file_info *fi);
int fuse7z_ftruncate(const char *path, off_t offset, struct fuse_file_info *fi);
int fuse7z_truncate(const char *path, off_t offset);
int fuse7z_unlink(const char *path);
int fuse7z_rmdir(const char *path);
int fuse7z_mkdir(const char *path, mode_t mode);
int fuse7z_rename(const char *path, const char *new_path);
int fuse7z_utimens(const char *path, const struct timespec tv[2]);
#if ( __FreeBSD__ >= 10 )
int fuse7z_setxattr(const char *, const char *, const char *, size_t, int, uint32_t);
int fuse7z_getxattr(const char *, const char *, char *, size_t, uint32_t);
#else
int fuse7z_setxattr(const char *, const char *, const char *, size_t, int);
int fuse7z_getxattr(const char *, const char *, char *, size_t);
#endif
int fuse7z_listxattr(const char *, char *, size_t);
int fuse7z_removexattr(const char *, const char *);
int fuse7z_chmod(const char *, mode_t);
int fuse7z_chown(const char *, uid_t, gid_t);
int fuse7z_flush(const char *, struct fuse_file_info *);
int fuse7z_fsync(const char *, int, struct fuse_file_info *);
int fuse7z_fsyncdir(const char *, int, struct fuse_file_info *);
int fuse7z_opendir(const char *, struct fuse_file_info *);
int fuse7z_releasedir(const char *, struct fuse_file_info *);
int fuse7z_access(const char *, int);


/**
 * Print usage information
 */
void print_usage() {
	fprintf(stderr, "usage: %s [options] <zip-file> <mountpoint>\n\n", PROGRAM);
	fprintf(stderr,
	  "general options:\n"
	  "	-o opt,[opt...]		mount options\n"
	  "	-h   --help			print help\n"
	  "	-V   --version		 print version\n"
	  "	-r   -o ro			 open archive in read-only mode\n"
	  "	-f					 don't detach from terminal\n"
	  "	-d					 turn on debugging, also implies -f\n"
	  "\n");
}

/**
 * Print version information (fuse-zip and FUSE library)
 */
void print_version() {
	fprintf(stderr, "%s version: %s\n", PROGRAM, VERSION);
}

/**
 * Parameters for command-line argument processing function
 */
struct fuse7z_param {
	int syslog;
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
	// int listing_progress_dot
	int dot;
	int automake;
	char mountpoint[4096];
} param;

#define KEY_HELP (0)
#define KEY_VERSION (1)
#define KEY_AUTO (2)
#define KEY_SYSLOG (3)

/**
 * Function to process arguments (called from fuse_opt_parse).
 *
 * @param data  Pointer to fuse7z_param structure
 * @param arg is the whole argument or option
 * @param key determines why the processing function was called
 * @param outargs the current output argument list
 * @return -1 on error, 0 if arg is to be discarded, 1 if arg should be kept
 */
static int process_arg(struct fuse7z_param *param, const char *arg, int key, struct fuse_args *outargs) {
	// 'magic' fuse_opt_proc return codes
	const static int KEEP = 1;
	const static int DISCARD = 0;
	const static int ERROR = -1;

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
		param->syslog = 1;
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
				fprintf(stderr, "%s: only two arguments allowed: filename and mountpoint\n", PROGRAM);
				return ERROR;
		}

		default:
		return KEEP;
	}
}

static const struct fuse_opt fuse7z_opts[] = {
	FUSE_OPT_KEY("-h", KEY_HELP),
	FUSE_OPT_KEY("--help", KEY_HELP),
	FUSE_OPT_KEY("-V", KEY_VERSION),
	FUSE_OPT_KEY("--version", KEY_VERSION),
	FUSE_OPT_KEY("--automount", KEY_AUTO),
	FUSE_OPT_KEY("--syslog", KEY_SYSLOG),
	{NULL, 0, 0}
};

struct fuse_operations const fuse7z_oper = {
	.init = fuse7z_init,
	.destroy = fuse7z_destroy,
	.readdir = fuse7z_readdir,
	.getattr = fuse7z_getattr,
	.statfs = fuse7z_statfs,
	.open = fuse7z_open,
	.read = fuse7z_read,
	.write = fuse7z_write,
	.release = fuse7z_release,
	.unlink = fuse7z_unlink,
	.rmdir = fuse7z_rmdir,
	.mkdir = fuse7z_mkdir,
	.rename = fuse7z_rename,
	.create = fuse7z_create,
	.chmod = fuse7z_chmod,
	.chown = fuse7z_chown,
	.flush = fuse7z_flush,
	.fsync = fuse7z_fsync,
	.fsyncdir = fuse7z_fsyncdir,
	.opendir = fuse7z_opendir,
	.releasedir = fuse7z_releasedir,
	.access = fuse7z_access,
	.utimens = fuse7z_utimens,
	.ftruncate = fuse7z_ftruncate,
	.truncate = fuse7z_truncate,
	.setxattr = fuse7z_setxattr,
	.getxattr = fuse7z_getxattr,
	.listxattr = fuse7z_listxattr,
	.removexattr = fuse7z_removexattr,
	#if FUSE_VERSION >= 28
	// don't allow NULL path
	.flag_nullpath_ok = 0,
	#endif
};


int main(int argc, char *argv[]) {
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	void * data;


	char *mountpoint;
	int multithreaded;	// this flag ignored because libzip does not supports multithreading
	int res;


	if (fuse_opt_parse(&args, &param, fuse7z_opts, (fuse_opt_proc_t)process_arg)) {
		fuse_opt_free_args(&args);
		return 2;
	}


	// if all work is done inside options parsing...
	if (param.help) {
		fuse_opt_free_args(&args);
		return 3;
	}
	

	// TODO...
	char * cwd = (char*)malloc(PATH_MAX + 1);
	if (getcwd(cwd, PATH_MAX) == NULL) {
		return 4;
	}

	if (!param.version) {
		// no file name passed
		if (param.fileName == NULL) {
			print_usage();
			fuse_opt_free_args(&args);
			return 4;
		}

		data = fuse7z_initlib(param.fileName, cwd);
		if (data == NULL) {
		  fuse_opt_free_args(&args);
		  return 5;
		}
	}

	if (param.automake) {
		mkdir(param.mountpoint, 0750);
	}

	struct fuse * fuse = fuse_setup(args.argc, args.argv, &fuse7z_oper, sizeof(fuse7z_oper), &mountpoint, &multithreaded, data);

	fuse_opt_free_args(&args);
	if (fuse == NULL) {
		return 7;
	}
	res = fuse_loop(fuse);

	fuse_teardown(fuse, mountpoint);

	if (param.automake) {
		rmdir(param.mountpoint);
	}

	free(cwd);
	return (res == 0) ? 0 : 8;
}

