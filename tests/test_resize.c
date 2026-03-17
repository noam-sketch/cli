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
    gtk_widget_set_vexpand(term1, TRUE);
    gtk_widget_set_hexpand(term1, TRUE);

    GtkWidget *term2 = vte_terminal_new();
    gtk_widget_set_size_request(term2, 1, 1);
    gtk_widget_set_vexpand(term2, TRUE);
    gtk_widget_set_hexpand(term2, TRUE);

    gtk_paned_pack1(GTK_PANED(paned), term1, TRUE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned), term2, TRUE, TRUE);

    gtk_widget_show_all(window);
    
    // Spawn simple commands to see them
    char *cmd[] = {"/bin/sh", NULL};
    vte_terminal_spawn_sync(VTE_TERMINAL(term1), VTE_PTY_DEFAULT, NULL, cmd, NULL, G_SPAWN_DEFAULT, NULL, NULL, NULL, NULL, NULL);
    vte_terminal_spawn_sync(VTE_TERMINAL(term2), VTE_PTY_DEFAULT, NULL, cmd, NULL, G_SPAWN_DEFAULT, NULL, NULL, NULL, NULL, NULL);

    // Run main loop for a short time or require user interaction if we were interactive.
    // For this context, we just want to compile and verify GTK APIs.
    
    return 0;
}
