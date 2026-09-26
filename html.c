#include <ctype.h>
#include <libxml/HTMLparser.h>
#include <libxml/tree.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	char *data;
	size_t length;
	size_t capacity;
} text_buffer;

static int append_char(text_buffer *buffer, char character)
{
	if (buffer->length + 2 > buffer->capacity) {
		size_t capacity = buffer->capacity == 0 ? 4096 : buffer->capacity * 2;
		char *data = realloc(buffer->data, capacity);
		if (data == NULL)
			return 0;
		buffer->data = data;
		buffer->capacity = capacity;
	}
	buffer->data[buffer->length++] = character;
	buffer->data[buffer->length] = '\0';
	return 1;
}

static int append_text(text_buffer *buffer, const char *text, size_t length)
{
	for (size_t i = 0; i < length; i++) {
		char character = text[i];
		if (character == '\n') {
			while (buffer->length > 0 &&
			       buffer->data[buffer->length - 1] == ' ')
				buffer->length--;
			if (buffer->length == 0 ||
			    buffer->data[buffer->length - 1] != '\n') {
				if (!append_char(buffer, '\n'))
					return 0;
			}
			continue;
		}
		if (isspace((unsigned char)text[i])) {
			if (buffer->length > 0 &&
			    buffer->data[buffer->length - 1] != ' ' &&
			    buffer->data[buffer->length - 1] != '\n')
				character = ' ';
			else
				continue;
		}

		if (!append_char(buffer, character))
			return 0;
	}
	return 1;
}

static int newline(text_buffer *buffer)
{
	if (buffer->length == 0 || buffer->data[buffer->length - 1] == '\n')
		return 1;
	while (buffer->length > 0 && buffer->data[buffer->length - 1] == ' ')
		buffer->length--;
	return append_char(buffer, '\n');
}

static int resolve_link(const char *base, const char *href,
			char *output, size_t output_size)
{
	if (href == NULL || href[0] == '\0' || href[0] == '#')
		return 0;
	if (strncmp(href, "javascript:", 11) == 0 ||
	    strncmp(href, "mailto:", 7) == 0)
		return 0;
	if (strncmp(href, "http://", 7) == 0 ||
	    strncmp(href, "https://", 8) == 0) {
		snprintf(output, output_size, "%s", href);
		return validate_url(output);
	}
	if (href[0] == '/') {
		const char *scheme_end = strstr(base, "://");
		if (scheme_end == NULL)
			return 0;
		const char *host_end = strchr(scheme_end + 3, '/');
		size_t prefix = host_end == NULL ? strlen(base) :
			(size_t)(host_end - base);
		if (prefix + strlen(href) + 1 > output_size)
			return 0;
		memcpy(output, base, prefix);
		snprintf(output + prefix, output_size - prefix, "%s", href);
		return validate_url(output);
	}

	const char *last_slash = strrchr(base, '/');
	size_t prefix = last_slash == NULL ? strlen(base) : (size_t)(last_slash - base + 1);
	if (prefix + strlen(href) + 1 > output_size)
		return 0;
	memcpy(output, base, prefix);
	snprintf(output + prefix, output_size - prefix, "%s", href);
	return validate_url(output);
}

static int visit(xmlNode *node, text_buffer *buffer, const char *base_url,
		 int tab_index)
{
	for (xmlNode *current = node; current != NULL; current = current->next) {
		int heading_level = 0;
		int block = current->type == XML_ELEMENT_NODE &&
			(xmlStrcasecmp(current->name, (xmlChar *)"body") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"header") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"main") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"nav") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"article") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"section") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"footer") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"p") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"div") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"ul") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"ol") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"li") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"table") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"title") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"h1") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"h2") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"h3") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"h4") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"tr") == 0 ||
			 xmlStrcasecmp(current->name, (xmlChar *)"br") == 0);
		if (current->type == XML_ELEMENT_NODE) {
			if (xmlStrcasecmp(current->name, (xmlChar *)"title") == 0 ||
			    xmlStrcasecmp(current->name, (xmlChar *)"h1") == 0)
				heading_level = 1;
			else if (xmlStrcasecmp(current->name, (xmlChar *)"h2") == 0)
				heading_level = 2;
			else if (xmlStrcasecmp(current->name, (xmlChar *)"h3") == 0 ||
				 xmlStrcasecmp(current->name, (xmlChar *)"h4") == 0)
				heading_level = 3;
		}
		int is_link = current->type == XML_ELEMENT_NODE &&
			xmlStrcasecmp(current->name, (xmlChar *)"a") == 0;

		if (current->type == XML_ELEMENT_NODE &&
		    (xmlStrcasecmp(current->name, (xmlChar *)"script") == 0 ||
		     xmlStrcasecmp(current->name, (xmlChar *)"style") == 0))
			continue;
		if (block && !newline(buffer))
			return 0;
		if (heading_level > 0) {
			char prefix[8];
			snprintf(prefix, sizeof(prefix), "^%d ", heading_level);
			if (!append_text(buffer, prefix, strlen(prefix)))
				return 0;
		}
		if (current->type == XML_ELEMENT_NODE &&
		    xmlStrcasecmp(current->name, (xmlChar *)"li") == 0 &&
		    !append_text(buffer, "  - ", 4))
			return 0;
		if (current->type == XML_TEXT_NODE && current->content != NULL &&
		    !append_text(buffer, (char *)current->content,
				 strlen((char *)current->content)))
			return 0;
		if (current->children != NULL &&
		    !visit(current->children, buffer, base_url, tab_index))
			return 0;
		if (is_link && link_counts[tab_index] < MAX_LINKS) {
			xmlChar *href = xmlGetProp(current, (xmlChar *)"href");
			char resolved[100];
			int link_number = link_counts[tab_index];
			if (href != NULL && resolve_link(base_url, (char *)href,
							 resolved, sizeof(resolved))) {
				snprintf(links[tab_index][link_number],
					 sizeof(links[tab_index][link_number]), "%s", resolved);
				link_counts[tab_index]++;
				char marker[32];
				snprintf(marker, sizeof(marker), " [%d]", link_number + 1);
				if (!append_text(buffer, marker, strlen(marker))) {
					xmlFree(href);
					return 0;
				}
			}
			xmlFree(href);
		}
		if (block && !newline(buffer))
			return 0;
	}
	return 1;
}

char *parse_page(char *html_content, char *base_url, int tab_index)
{
	htmlDocPtr document;
	text_buffer buffer = {0};

	if (html_content == NULL)
		return NULL;
	link_counts[tab_index] = 0;
	document = htmlReadMemory(html_content, (int)strlen(html_content),
				  NULL, "UTF-8",
				  HTML_PARSE_RECOVER | HTML_PARSE_NOERROR |
				  HTML_PARSE_NOWARNING);
	if (document == NULL)
		return NULL;

	int success = visit(xmlDocGetRootElement(document), &buffer, base_url,
			    tab_index);
	xmlFreeDoc(document);
	if (!success) {
		free(buffer.data);
		return NULL;
	}
	if (buffer.data == NULL)
		buffer.data = calloc(1, 1);
	return buffer.data;
}

char *parse(char *html_content)
{
	return parse_page(html_content, "", cur);
}
