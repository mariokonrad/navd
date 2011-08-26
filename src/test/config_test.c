#include <stdio.h>
#include <stdlib.h>
#include <config/config.h>
#include <common/macros.h>

int main(int argc, char ** argv)
{
	int rc;
	size_t i;
	struct config_t config;

	UNUSED_ARG(argc);

	rc = config_parse_file(argv[1], &config);
	if (rc < 0) return EXIT_FAILURE;

	printf("===== SOURCES ========================\n");
	for (i = 0; i < config.num_sources; ++i) {
		struct source_t * p = &config.sources[i];
		printf("  %s : %s\n", p->name, p->type);
	}
	printf("===== DESTINATIONS ===================\n");
	for (i = 0; i < config.num_destinations; ++i) {
		struct destination_t * p = &config.destinations[i];
		printf("  %s : %s\n", p->name, p->type);
	}
	printf("===== FILTERS ========================\n");
	for (i = 0; i < config.num_filters; ++i) {
		struct filter_t * p = &config.filters[i];
		printf("  %s : %s\n", p->name, p->type);
	}
	printf("===== ROUTES =========================\n");
	for (i = 0; i < config.num_routes; ++i) {
		struct route_t * p = &config.routes[i];
		printf("  %s --[%s]--> %s\n", p->name_source, p->name_filter, p->name_destination);
	}

	config_free(&config);
	return EXIT_SUCCESS;
}

