void set_url(char *str) { /* we set url for later use */
	if (cur + 1 >= MAX_WEBSITES) {
		free(pages[0]);
		for (int i = 1; i < MAX_WEBSITES; i++) {
			strcpy(websites[i - 1], websites[i]);
			pages[i - 1] = pages[i];
		}
		cur = MAX_WEBSITES - 2;
		wmax = MAX_WEBSITES - 2;
	}
	cur++;
	strncpy(websites[cur], str, sizeof(websites[cur]) - 1);
	websites[cur][sizeof(websites[cur]) - 1] = '\0';
	pages[cur] = NULL;
	wmax = wmax > cur ? wmax : cur;	/* changing wmax if needed */
}
/*
 * if we change	"strcpy(websites[++cur], str);" 
 * for "cur++; strcpy(websites[cur], str);"
 * it won't work
 * i don't know why
 */


char *get_url(void) { 	/* simply return url */
	if(cur < 0) return NULL; 	/* if no url-s yet, prevent errors */
	return websites[cur];		/* else return url */
}
