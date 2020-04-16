#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

struct uci_type_list {
	unsigned int idx;
	const char *name;
	struct uci_type_list *next;
};

static const char *delimiter = " ";
static const char *cur_section_ref = NULL;
static struct uci_type_list *type_list = NULL;
static char *typestr = NULL;

static char *uci_lookup_section_ref(struct uci_section *s)
{
	struct uci_type_list *ti = type_list;
	char *ret;
	int maxlen;

	/* look up in section type list */
	while (ti) {
		if (strcmp(ti->name, s->type) == 0)
			break;
		ti = ti->next;
	}
	if (!ti) {
		ti = malloc(sizeof(struct uci_type_list));
		if (!ti)
			return NULL;
		memset(ti, 0, sizeof(struct uci_type_list));
		ti->next = type_list;
		type_list = ti;
		ti->name = s->type;
	}

	if (s->anonymous) {
		maxlen = strlen(s->type) + 1 + 2 + 10;
		if (!typestr) {
			typestr = malloc(maxlen);
			if (!typestr)
				return NULL;
		} else {
			void *p = realloc(typestr, maxlen);
			if (!p) {
				free(typestr);
				return NULL;
			}

			typestr = p;
		}

		if (typestr)
			sprintf(typestr, "@%s[%d]", ti->name, ti->idx);

		ret = typestr;
	} else {
		ret = s->e.name;
	}

	ti->idx++;

	return ret;
}

static void uci_print_value(FILE *f, const char *v)
{
	fprintf(f, "'");
	while (*v) {
		if (*v != '\'')
			fputc(*v, f);
		else
			fprintf(f, "'\\''");
		v++;
	}
	fprintf(f, "'");
}

static void uci_show_value(struct uci_option *o, bool quote)
{
	struct uci_element *e;
	bool sep = false;
	char *space;

	switch(o->type) {
	case UCI_TYPE_STRING:
		if (quote)
			uci_print_value(stdout, o->v.string);
		else
			printf("%s", o->v.string);
		printf("\n");
		break;
	case UCI_TYPE_LIST:
		uci_foreach_element(&o->v.list, e) {
			printf("%s", (sep ? delimiter : ""));
			space = strpbrk(e->name, " \t\r\n");
			if (!space && !quote)
				printf("%s", e->name);
			else
				uci_print_value(stdout, e->name);
			sep = true;
		}
		printf("\n");
		break;
	default:
		printf("<unknown>\n");
		break;
	}
}

static void uci_show_option(struct uci_option *o, bool quote)
{
	printf("%s.%s.%s=",
		o->section->package->e.name,
		(cur_section_ref ? cur_section_ref : o->section->e.name),
		o->e.name);
	uci_show_value(o, quote);
}

static void uci_show_section(struct uci_section *s)
{
	struct uci_element *e;
	const char *cname;
	const char *sname;

	cname = s->package->e.name;
	sname = (cur_section_ref ? cur_section_ref : s->e.name);
	printf("%s.%s=%s\n", cname, sname, s->type);
	uci_foreach_element(&s->options, e) {
		uci_show_option(uci_to_option(e), true);
	}
}

static void uci_reset_typelist(void)
{
	struct uci_type_list *type;
	while (type_list != NULL) {
			type = type_list;
			type_list = type_list->next;
			free(type);
	}
	if (typestr) {
		free(typestr);
		typestr = NULL;
	}
	cur_section_ref = NULL;
}

void uci_show_package(struct uci_package *p)
{
	struct uci_element *e;

	uci_reset_typelist();
	uci_foreach_element( &p->sections, e) {
		struct uci_section *s = uci_to_section(e);
		cur_section_ref = uci_lookup_section_ref(s);
		uci_show_section(s);
	}
	uci_reset_typelist();
}

