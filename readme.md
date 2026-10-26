# ABOB — Another Buggy, Open Browser

A small **text-mode web browser written in C**, with `ncurses` for the TUI and
Vim-style keybindings. An early, educational project: it fetches a page over
`libcurl`, parses the HTML with `libxml2`, and lays the text out in the terminal.

> Written in 2023 together with [@maxim3007](https://github.com/maxim3007) as a
> way to learn C, `ncurses` and the HTTP/HTML stack. It is rough and incomplete
> — kept here as a learning artifact, not a finished browser.

## Keybindings

| Key | Action |
| --- | --- |
| `:` | Prompt mode — type a URL or command |
| `i` | Insert mode (text entry) |
| `y` | Yank the current URL |
| `Tab` | Cycle through open tabs |
| `Esc` | Leave the current mode |

## Build

Dependencies: a C compiler, `ncurses`, `libcurl`, and `libxml2`.

```bash
# Debian/Ubuntu
sudo apt install build-essential libncurses-dev libcurl4-openssl-dev libxml2-dev

make        # builds ./abob
./abob
```

`make` wraps the compile in [`build.sh`](build.sh); edit either if your system
puts `libxml2` headers somewhere other than `/usr/include/libxml2`.

## Project layout

| File | What it does |
| --- | --- |
| `abob.c` | Entry point and the main input loop |
| `request.c` | HTTP requests and URL validation (libcurl) |
| `html.c` | HTML parsing and text extraction (libxml2) |
| `manage_url.c` | Current-URL state |
| `start_page.c` | The welcome screen |
| `prompt_mode.c` | Prompt / insert input handling |

## Status / TODO

- [x] Fetch a page and render its text
- [x] Prompt and insert modes, tabs, yank
- [ ] Proper page rendering (links, structure) — currently text only
- [ ] Handle content wider than the viewport
- [ ] In-app help page

## License

MIT — see [LICENSE](LICENSE).
