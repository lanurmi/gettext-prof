/* Dynamic xgettext / profiler tool.
 *
 * (C) Lauri Nurmi <lanurmi@iki.fi>, 2019 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

char *(*old_gettext) (const char *__msgid);
char *(*old_dngettext) (const char *__domainname, const char *__msgid1,
                        const char *__msgid2, unsigned long int __n);
char *(*old_dcgettext) (const char *__domainname,
                        const char *__msgid, int __category);

static char *g_messages[65536] = {0};
static char *g_plural_messages[65536] = {0};
static int g_nextIndex;

static const char *escape_out(FILE *out, const char *msg) {
	const char *p = msg;
	while(*p) {
		if (*p == '\n') {
			fputs("\\n", out);
			if (*(p+1))
				fputs("\"\n\"", out);
		}
		else if (*p == '\"')
			fputs("\\\"", out);
		else if (*p == '\\')
			fputs("\\\\", out);
		else
			fputc(*p, out);
		++p;
	}
	return msg;
}

enum PotContent { Header = 0x1, Body = 0x2 };

static void output_message(FILE *sink, const char *msg, const char *msgP) {
	fprintf(sink,
	    "msgid \"");
	if (strlen(msg) > 70 ||
	    strchr(msg, '\n') != NULL)
		fprintf(sink, "\"\n\"");
	escape_out(sink, msg);
	fprintf(sink, "\"\n");
	if (msgP) {
		fprintf(sink, "msgid_plural \"");
		escape_out(sink, msgP);
		fprintf(sink, "\"\n"
		    "msgstr_plural[0] \"\"\n"
		    "msgstr_plural[1] \"\"\n\n");
	} else {
		fprintf(sink, "msgstr \"\"\n\n");
	}
}

static void produce_pot_lines(FILE *sink, enum PotContent c) {
	if (c & Header) {
		fprintf(sink, "msgid \"\"\n"
			"msgstr \"\"\n\"");
		escape_out(sink, old_gettext(""));
		fprintf(sink, "\"\n\n");
	}
	if (c & Body) {
		for (int i = 0; i < g_nextIndex; ++i) {
			output_message(sink, g_messages[i], g_plural_messages[i]);
		}
	}
}

static int produce_filename(char *filename, size_t bufsize, enum PotContent c) {
	char buffer[2048];
	const char *dir = ".", *outdir = NULL;

	if ((outdir = getenv("POT_OUT"))) {
		mkdir(outdir, 0755);
		dir = outdir;
	}

	strncpy(buffer, old_gettext(""), sizeof(buffer));
	if (strstr(buffer, "Project-Id-Version: ") == buffer) {
		char *domainname = buffer + strlen("Project-Id-Version: ");
		char *p = strchr(domainname, '\n');
		if (p)
			*p = '\0';
		p = strchr(domainname, ' ');
		if (p)
			*p = '_';
		p = strchr(domainname, '/');
		if (p)
			*p = '_';

		if (c == Header) {
			snprintf(filename, bufsize, "%s/%s_header.pot", dir, domainname);
		} else if (c == Body) {
			snprintf(filename, bufsize, "%s/%s_textXXXXXX", dir, domainname);
		}

		return 0;
	} else {
		if (c == Header) {
			snprintf(filename, bufsize, "%s/%s_header.pot", dir, "unknown");
		} else if (c == Body) {
			snprintf(filename, bufsize, "%s/%s_textXXXXXX", dir, "unknown");
		}

		return -1;
	}
}

static void generate_all_pot_parts(void) {
	char headerfname[2048];
	produce_filename(headerfname, sizeof(headerfname), Header);
	struct stat s;
	if (stat(headerfname, &s) != 0) {
		FILE *header = fopen(headerfname, "wt");
		produce_pot_lines(header, Header);
		fclose(header);
	}
	char filename[2048];
	produce_filename(filename, sizeof(filename), Body);
	int fd = mkstemp(filename);
	if (fd < 0)
		perror("mkstemp");
	FILE *pot = fdopen(fd, "wt");
	produce_pot_lines(pot, Body);
	fclose(pot);
}

static void finish() {
	generate_all_pot_parts();
	for (int i = 0; i < g_nextIndex; ++i) {
		free(g_messages[i]);
		free(g_plural_messages[i]);
	}
	g_nextIndex = 0;
}

static void init() {
	if (old_gettext != NULL)
		return;

	atexit(finish);
	old_gettext = dlsym(RTLD_NEXT, "gettext");
	old_dngettext = dlsym(RTLD_NEXT, "dngettext");
	old_dcgettext = dlsym(RTLD_NEXT, "dcgettext");
}

static void use(const char *msg, const char *msgPlural) {
	if ((size_t)g_nextIndex == sizeof(g_messages) / sizeof(*g_messages)) {
		fprintf(stderr, "Limit for unique messages reached! POT generation stopped.\n");
		return;
	}

	for (int i = 0; i < g_nextIndex; ++i) {
		if (strcmp(g_messages[i], msg) == 0) {
			return;
		}
	}

	g_messages[g_nextIndex] = malloc(1 + strlen(msg));
	if (msgPlural) {
		g_plural_messages[g_nextIndex] = malloc(1 + strlen(msgPlural));
		strcpy(g_plural_messages[g_nextIndex], msgPlural);
	}

	strcpy(g_messages[g_nextIndex++], msg);
}

char *gettext(const char *msgid) {
	init();
	use(msgid, NULL);
	return old_gettext(msgid);
}

char *dngettext(const char *domainname, const char *msgid1,
		const char *msgid2, unsigned long int n) {
	init();
	use(msgid1, msgid2);
	return old_dngettext(domainname, msgid1, msgid2, n);
}

char *dcgettext(const char *domainname, const char *msgid,
		int category) {
	init();
	use(msgid, NULL);
	return old_dcgettext(domainname, msgid, category);
}

