#include <gtk/gtk.h>
#include <vte/vte.h>

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_container_add(GTK_CONTAINER(window), paned);

    GtkWidget *term1 = vte_terminal_new();
    gtk_widget_set_size_request(term1, 1, 1);
    GtkWidget *term2 = vte_terminal_new();
    gtk_widget_set_size_request(term2, 1, 1);

    gtk_paned_pack1(GTK_PANED(paned), term1, TRUE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned), term2, TRUE, TRUE);

    gtk_widget_show_all(window);
    
    // We want to see what position it gets
    g_print("Paned position before loop: %d\n", gtk_paned_get_position(GTK_PANED(paned)));
    
    return 0;
}
