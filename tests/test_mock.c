#include <gtk/gtk.h>
#include <vte/vte.h>
#define vte_terminal_copy_clipboard_format mock_copy
void mock_copy(VteTerminal *term, VteFormat format) {}
#include "../main.c"
int main() { return 0; }
