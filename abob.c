/*
 * Main script
 */

#include "abob.h"			/* move declarations to another file */

/* include other files */
#include "request.c"			/* requesting html and validating urls */
#include "html.c"			/* only max is able to understand this */
#include "manage_url.c"			/* tools to work with current url */
#include "start_page.c"			/* basic welcome page */
#include "prompt_mode.c"		/* working with prompt */

static void render_page(WINDOW *window, const char *text);

int main(){
	int ch;
	initscr();
	curl_global_init(CURL_GLOBAL_DEFAULT);
	noecho(); 		/* turning off echoing input */
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(0);		/* turning off cursor */
	refresh();		/* refresh to see changes */
	open_start();		/* opening welcome page	*/


	do{					/* processing input */
		ch = getch();

		switch(ch){
			case ':': 		/* enter prompt mode*/
				open_prompt();
				break;
			case 'y':		/* yank current url */
				clear();
				char *current_url = get_url();
				printw("Your clipboard is %s",
					current_url == NULL ? "(no website opened)" : current_url);
				refresh();
				break;
			case 'i':		/* enter insert mode */
				open_insert();
				break;
			case 't':		/* delete? */
				open_tabs();
				break;
			case 9: /* tab key */	/* scroll tabs */
				if (cur >= 0) {
					scroll_tabs();
					open_tabs();
					if (web_wins[cur] != NULL) {
						if (pages[cur] != NULL)
							render_page(web_wins[cur], pages[cur]);
					}
				}
				break;
			case '1': case '2': case '3': case '4': case '5':
			case '6': case '7': case '8': case '9':
				open_link(ch - '0');
				break;
			default:
				break;
		}
		upd_tab_bar(); /* after each action update/add tabs */
	}while(true);

	for (int i = 0; i <= wmax; i++)
		free(pages[i]);
	curl_global_cleanup();
	endwin();
	return 0;
}

void open_insert(){
	ins = newwin(0, 0, 0, 0);
	if (check_win(ins)) return;
	
	curs_set(1);	/* turning cursor on while in insert mode */
	wrefresh(ins);
	for (;;) {
	    int ch = getch();
	    waddch(ins, ch);
	    switch(ch){
		case 27:/* escape button */
		    clear(); refresh(); 
		    curs_set(0);return; /* exit insert mode */
		default:
		    break;
	     }
	     wrefresh(ins);
	}
	clear();
       	refresh();
	curs_set(0);
}

/* some web stuff, added by max */
void tag_callback(char *tagname){
	printf("%s", tagname); 
}

static void render_page(WINDOW *window, const char *text)
{
	int height, width;
	int row = 0;
	const char *line = text;
	getmaxyx(window, height, width);
	werase(window);
	if (width < 2 || height < 1) {
		wrefresh(window);
		return;
	}

	while (*line != '\0' && row < height) {
		const char *end = strchr(line, '\n');
		size_t length = end == NULL ? strlen(line) : (size_t)(end - line);
		int heading = 0;
		const char *content = line;

		if (length >= 3 && line[0] == '^' &&
		    line[1] >= '1' && line[1] <= '3') {
			heading = line[1] - '0';
			content += 3;
			length -= 3;
		}

		if (length > 0) {
			while (length > 0 && row < height) {
				size_t chunk = length < (size_t)(width - 1) ?
					length : (size_t)(width - 1);
				if (chunk < length) {
					size_t split = chunk;
					while (split > 0 && content[split] != ' ')
						split--;
					if (split > 0)
						chunk = split;
				}
				if (heading > 0)
					wattron(window, A_BOLD |
						(heading == 1 ? A_UNDERLINE : 0));
				mvwaddnstr(window, row++, 0, content, (int)chunk);
				if (heading > 0)
					wattroff(window, A_BOLD |
						(heading == 1 ? A_UNDERLINE : 0));
				content += chunk;
				length -= chunk;
				while (length > 0 && *content == ' ') {
					content++;
					length--;
				}
			}
		} else {
			row++;
		}

		if (end == NULL)
			break;
		line = end + 1;
	}
	wrefresh(window);
}


void render_website(char *url){
	wclear(web_wins[cur]);
	wprintw(web_wins[cur], "Opening %s...\n\n", url);
	char *html = request(url);
	if (html == NULL) {
		wprintw(web_wins[cur], "Unable to load this website.\n");
		wrefresh(web_wins[cur]);
		return;
	}
	free(pages[cur]);
	pages[cur] = parse_page(html, websites[cur], cur);
	if (pages[cur] == NULL) {
		wprintw(web_wins[cur], "Unable to read the page content.\n");
	} else {
		render_page(web_wins[cur], pages[cur]);
		if (link_counts[cur] > 0) {
			wprintw(web_wins[cur], "\nLinks: ");
			for (int i = 0; i < link_counts[cur]; i++)
				wprintw(web_wins[cur], "[%d] %s  ", i + 1, links[cur][i]);
			wprintw(web_wins[cur], "\nType :link <number> to open a link.\n");
		}
	}
	wrefresh(web_wins[cur]);
}

void open_website(char *site){
	char url[sizeof(websites[0])];
	if (strncmp(site, "http://", 7) != 0 &&
	    strncmp(site, "https://", 8) != 0) {
		snprintf(url, sizeof(url), "https://%s", site);
		site = url;
	}

	int validity = validate_url(site);
	if (validity){
		set_url(site);
		web_wins[cur] = newwin(0, 0, 1, 0); /* everything but first line of screen */
		if (check_win(web_wins[cur])) return;

		wprintw(web_wins[cur], "Opening %s...", site);
		wrefresh(web_wins[cur]);
		render_website(site);
	} else {
		clear();
		printw("Error: Url %s is not valid!", site);
		refresh();
	}
}

void open_link(int number)
{
	if (cur < 0 || number < 1 || number > link_counts[cur]) {
		if (cur >= 0 && web_wins[cur] != NULL)
			wprintw(web_wins[cur], "\nInvalid link number.\n");
		return;
	}
	open_website(links[cur][number - 1]);
}
/* end of web stuff */

void open_tabs(){
	if (t_bar == NULL)
		t_bar = newwin(1, 0, 0, 0); 	/* first line of screen */
	if (check_win(t_bar)) return;
	wclear(t_bar);

	if (wmax < 0) {
		wrefresh(t_bar);
		return;
	}

	int i = 0;
	for (i = 0; i <= wmax; i++){ 		/* for every website url */
	    wprintw(t_bar, "| ");
	    if (i == cur) {
		wattron(t_bar, A_STANDOUT);	 /* highlight current website */ 
		wprintw(t_bar, "%s", websites[i]); /* print its name */
		wattroff(t_bar, A_STANDOUT);
		waddch(t_bar, ' ');
	    } else {
		wprintw(t_bar, "%s", websites[i]);
		waddch(t_bar, ' ');
	    }
	}
	waddstr(t_bar, "|");
	wrefresh(t_bar);
} 

void wcenter(WINDOW *win, int row, char *title){
	int len, indent, y, width;		
       	getmaxyx(win, y, width);		/* get coords of window */

	len = strlen(title);			/* getting len of the given string */
	indent = width - len;			/* counting how much space  */
	indent /= 2;				/* will we need to retreat  */
	mvwaddstr(win, row, indent, title);	/* finally print message    */
	wrefresh(win);				/* and refresh given window */
}


char *get_thing_after_space(char *text) {
	char* space_ptr = strstr(text, " "); 	/* set pointer for first space */
	if(space_ptr == NULL) return ""; 	/* if nothing after space, exit */
	space_ptr++;				/* do not include space */
	return space_ptr;			/* finally return */
}
bool check_win(WINDOW *win){
	if(win == NULL){
	    addstr("Error while allocating memory for window");
	    endwin();
	    return true;
	} else {
	    return false;
	}
}

void upd_tab_bar(){
	open_tabs();
	if (t_bar != NULL) {
		touchwin(t_bar);
		wrefresh(t_bar);
	}
}

void scroll_tabs(void){
	/* 
	 * simple loop:
	 * if it hasn't reached it limits, add 1
	 * else go to the beginning 
	 */
	if(cur < wmax){
	    cur++;
	} else {
	    cur = 0;
	}
}
