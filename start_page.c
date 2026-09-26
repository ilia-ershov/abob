void open_start(){
	int height, width, y;
	start = stdscr;
	werase(start);

	getmaxyx(start, height, width);
	y = height >> 1; /* center of y axis */

	wcenter(start, y-3, "Welcome to abob");
	wcenter(start, y-2, "ABOB - Another Buggy, Open Browser");
	wcenter(start, y-1, "version 0.0.0.6");
	wcenter(start, y+0, "");
	wcenter(start, y+1, "type :o 'website name' <enter> to open a website\n");
	wcenter(start, y+2, "type :q <enter> to exit abob\n");
	wcenter(start, y+3, "type :help for help\n");
	wcenter(start, y+4, "type :link N to follow a page link\n");

        touchwin(start);
	wrefresh(start);
}
