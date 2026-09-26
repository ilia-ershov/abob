/*
 * Opening prompt and getting user commands
 */

/* this is local so i won't add it to header file */
void show_error(char *err_text);

void open_prompt(){
	int height, width;
	getmaxyx(stdscr, height, width);
	prompt = newwin(1, 0, height - 1, 0); /* place prompt in the bottom of screen */
	if (check_win(prompt)) return;

	curs_set(1);		/* turning cursor on while in prompt mode */

	waddstr(prompt, ":");

        touchwin(prompt);
	wrefresh(prompt);
	
	echo();		/* add echo so user will be able to see his commands */
	char all[256];
	wgetnstr(prompt, all, 256); /* get commands from user */
	

	/* if user doesn't enter anything programm will crash, prevent this */
	if(strlen(all) == 0) 
	{curs_set(0); noecho(); return;} /* turn off prompt if there are 0 commands */

	
	char *content = get_thing_after_space(all);	/* get text after command */
	char *cmd = strtok(all, " ");			/* get user command */
	
	if(strcmp(cmd, "o") == 0 || strcmp(cmd, "open") == 0){	/* open website */
		if(content[0] == '\0') /* if user doesn't open anything show error */
			show_error("Error! Nothing to open");
		else
			open_website(content);
	}else if(strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0){	/* quit */
		endwin();
		exit(0);
	}else if(strcmp(cmd, "help") == 0 || strcmp(cmd, "home") == 0){
		open_start();
	}else if(strcmp(cmd, "link") == 0){
		char *end;
		long number = strtol(content, &end, 10);
		if (content[0] == '\0' || *end != '\0' || number < 1 ||
		    number > MAX_LINKS)
			show_error("Usage: :link <number>");
		else
			open_link((int)number);
	}else{
		show_error("Error! Command not found");
	}

	curs_set(0);	/* turning off cursor */
	noecho();	/* turn off echo */ 
}
void show_error(char *err_text){
	wclear(prompt);
	waddstr(prompt, err_text);
	wrefresh(prompt);
}
