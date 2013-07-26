#include <global_config.h>
#include <programoptions.h>
#include <common/macros.h>
#include <getopt.h>
#include <string.h>
#include <stdlib.h>
#include <syslog.h>

#ifdef ENABLE_FILTER_LUA
	#include <navcom/filter/filter_lua.h>
#endif

#ifdef ENABLE_DESTINATION_LUA
	#include <navcom/destination/dst_lua.h>
#endif

#ifdef ENABLE_SOURCE_LUA
	#include <navcom/source/src_lua.h>
#endif
/**
 * Enumeration of all program options.
 */
enum {
	 OPTION_HELP = 1000
	,OPTION_VERSION
	,OPTION_DEAMON
	,OPTION_CONFIG
	,OPTION_LIST
	,OPTION_LIST_COMPACT
	,OPTION_DUMP_CONFIG
	,OPTION_MAX_MSG
	,OPTION_LOG
};

static const struct option OPTIONS_LONG[] =
{
	{ "help",         optional_argument, 0, OPTION_HELP         },
	{ "version",      no_argument,       0, OPTION_VERSION      },
	{ "daemon",       no_argument,       0, OPTION_DEAMON       },
	{ "config",       required_argument, 0, OPTION_CONFIG       },
	{ "list",         no_argument,       0, OPTION_LIST         },
	{ "list-compact", no_argument,       0, OPTION_LIST_COMPACT },
	{ "dump-config",  no_argument,       0, OPTION_DUMP_CONFIG  },
	{ "max-msg",      required_argument, 0, OPTION_MAX_MSG      },
	{ "log",          required_argument, 0, OPTION_LOG          },
};

static void print_version(void)
{
	printf("%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

static void print_config(const char * prefix, const char * suffix)
{
#if defined(ENABLE_SOURCE_LUA)
	printf("%ssrc_lua(%s)%s", prefix, src_lua_release(), suffix);
#endif

#if defined(ENABLE_SOURCE_GPSSERIAL)
	printf("%sgpsserial%s", prefix, suffix);
#endif

#if defined(ENABLE_SOURCE_GPSSIMULATOR)
	printf("%sgpssimulator%s", prefix, suffix);
#endif

#if defined(ENABLE_SOURCE_SEATALKSIMULATOR)
	printf("%sseatalksimulator%s", prefix, suffix);
#endif

#if defined(ENABLE_SOURCE_SEATALKSERIAL)
	printf("%sseatalkserial%s", prefix, suffix);
#endif

#if defined(ENABLE_SOURCE_GPSD)
	printf("%sgpsd%s", prefix, suffix);
#endif

#if defined(ENABLE_FILTER_LUA)
	printf("%sfilter_lua(%s)%s", prefix, filter_lua_release(), suffix);
#endif

#if defined(ENABLE_DESTINATION_LUA)
	printf("%sdst_lua(%s)%s", prefix, dst_lua_release(), suffix);
#endif
}

void print_usage(const char * name)
{
	printf("\n");
	printf("usage: %s [options]\n", name);
	printf("\n");
	printf("Version: ");
	print_version();
	printf("\n");
	printf("Configured features:\n");
	print_config("  - ", "\n");
	printf("\n");
	printf("Options:\n");
	printf("  --help [=what]  : help information, optional on a source, filter or destination\n");
	printf("  --version       : version information\n");
	printf("  --daemon        : daemonize process\n");
	printf("  --config file   : configuration file\n");
	printf("  --list          : lists all sources, destinations and filters\n");
	printf("  --list-compact  : lists all sources, destinations and filters on one compact line\n");
	printf("  --dump-config   : dumps the configuration and exit\n");
	printf("  --max-msg n     : routes n number of messages before terminating\n");
	printf("  --log n         : defines log level on syslog (0..7)\n");
	printf("\n");
}

int parse_options(
		int argc,
		char ** argv,
		struct options_data_t * options)
{
	int rc;
	int index;
	char * endptr = NULL;

	/* default values */
	memset(options, 0, sizeof(struct options_data_t));
	options->log_mask = LOG_DEBUG;

	while (1) {
		rc = getopt_long(argc, argv, "", OPTIONS_LONG, &index);
		if (rc == -1)
			break;
		switch (rc) {
			case OPTION_HELP:
				options->help = 1;
				if (optarg)
					strncpy(options->help_specific, optarg, sizeof(options->help_specific));
				return 0;
			case OPTION_VERSION:
				print_version();
				return -1;
			case OPTION_DEAMON:
				options->daemonize = 1;
				break;
			case OPTION_CONFIG:
				options->config = 1;
				strncpy(options->config_filename, optarg, sizeof(options->config_filename)-1);
				break;
			case OPTION_LIST:
				options->list = 1;
				break;
			case OPTION_LIST_COMPACT:
				options->list_compact = 1;
				break;
			case OPTION_DUMP_CONFIG:
				options->dump_config = 1;
				break;
			case OPTION_MAX_MSG:
				options->max_msg = strtoul(optarg, &endptr, 0);
				if (*endptr != '\0') {
					syslog(LOG_ERR, "invalid value for parameter '%s': '%s'", OPTIONS_LONG[index].name, optarg);
					return -1;
				}
				break;
			case OPTION_LOG:
				options->log_mask = strtoul(optarg, &endptr, 0);
				if (*endptr != '\0') {
					syslog(LOG_ERR, "invalid value for parameter '%s': '%s'", OPTIONS_LONG[index].name, optarg);
					return -1;
				}
				options->log_mask = max(LOG_EMERG, options->log_mask);
				options->log_mask = min(LOG_DEBUG, options->log_mask);
				break;
			default:
				break;
		}
	}
	if (optind < argc) {
		syslog(LOG_ERR, "unknown parameters");
		print_usage(argv[0]);
		return -1;
	}
	return 0;
}

